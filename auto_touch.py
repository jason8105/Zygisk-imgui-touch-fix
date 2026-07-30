import os
import time
import subprocess
import requests
import re
import json

# API Keys pool
API_KEYS_POOL = {
    1: os.environ.get("GEMINI_KEY_1", ""),
    2: os.environ.get("GEMINI_KEY_2", "")
}

def load_highest_token_models():
    """Automatically reads allmodelai.json and picks ALL text models sorted by highest inputTokenLimit"""
    try:
        if os.path.exists("allmodelai.json"):
            with open("allmodelai.json", "r", encoding="utf-8") as f:
                data = json.load(f)
                models_list = data.get("models", [])
                
                valid_models = []
                for m in models_list:
                    methods = m.get("supportedGenerationMethods", [])
                    if "generateContent" in methods:
                        valid_models.append({
                            "name": m.get("name"),
                            "tokens": m.get("inputTokenLimit", 0)
                        })
                
                valid_models.sort(key=lambda x: x["tokens"], reverse=True)
                all_models = [item["name"] for item in valid_models]
                if all_models:
                    print(f"[*] Auto-loaded ALL models from JSON (Highest to Lowest): {all_models}")
                    return all_models
    except Exception as e:
        print(f"[!] Error reading JSON model list: {e}")
    
    return [
        "models/gemini-2.5-flash",
        "models/gemini-2.5-pro",
        "models/gemini-2.0-flash"
    ]

MODELS_POOL = load_highest_token_models()

# ==========================================
# MANUAL REPOSITORY SELECTION
# ==========================================
print("\n==================================================")
print(" Full-Auto Zygisk Builder & Touch Fixer")
print("==================================================")
_owner = input("Enter GitHub Username/Owner (Press Enter for 'jason8105'): ").strip()
REPO_OWNER = _owner if _owner else "jason8105"

_repo = input("Enter Repository Name (Press Enter for 'Zygisk-imgui-touch-fix'): ").strip()
REPO_NAME = _repo if _repo else "Zygisk-imgui-touch-fix"

print(f"\n[*] Target Repository set to: {REPO_OWNER}/{REPO_NAME}")
print("==================================================\n")
# ==========================================

GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")

HEADERS = {
    "Authorization": f"Bearer {GITHUB_TOKEN}",
    "Accept": "vnd.github+json"
} if GITHUB_TOKEN else {}

def run_cmd(cmd):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.stdout + result.stderr

def trigger_workflow_dispatch():
    url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/actions/workflows/build.yml/dispatches"
    try:
        requests.post(url, headers=HEADERS, json={"ref": "main"}, timeout=10)
    except Exception:
        pass

def get_latest_workflow_run():
    url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/actions/runs?per_page=1"
    try:
        res = requests.get(url, headers=HEADERS, timeout=15).json()
        runs = res.get("workflow_runs", [])
        if not runs:
            return None, None
        return runs[0]["id"], runs[0]["status"]
    except Exception:
        return None, None

def get_workflow_logs(run_id, max_retries=5):
    print("[*] Fetching failed job details to get direct text logs...")
    jobs_url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/actions/runs/{run_id}/jobs"

    for attempt in range(max_retries):
        try:
            res = requests.get(jobs_url, headers=HEADERS, timeout=30)
            if res.status_code != 200:
                print(f"[!] Failed to get jobs. Status {res.status_code}. Retrying...")
                time.sleep(5)
                continue

            jobs = res.json().get("jobs", [])
            all_logs = ""

            for job in jobs:
                if job.get("conclusion") == "failure":
                    job_id = job["id"]
                    print(f"[*] Downloading text log for failed job: {job['name']}...")
                    log_url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/actions/jobs/{job_id}/logs"

                    log_res = requests.get(log_url, headers=HEADERS, allow_redirects=True, timeout=60)
                    if log_res.status_code == 200:
                        all_logs += f"\n=== Job: {job['name']} ===\n" + log_res.text
                        print(f"[*] Log downloaded successfully for {job['name']}")
                    else:
                        print(f"[!] Failed to get log text. Status {log_res.status_code}")

            if all_logs:
                return all_logs
            else:
                return "Empty logs or no failure found."

        except Exception as e:
            print(f"[!] Network error fetching job logs: {e}. Retrying ({attempt+1}/{max_retries})...")
            time.sleep(10)

    return "Log fetch error: Network timeout."

def ask_gemini_http(error_logs):
    prompt = f"""
You are an elite Android NDK, C++, Gradle, and Zygisk module build engineer.
Your mission is to ensure this repository compiles into a fully functional, flashable Magisk Zygisk module zip that contains a Unity JNI touch-fixed ImGui menu.

CRITICAL REQUIREMENTS:
1. TOUCH FIX: You must completely remove any standard native input hooks (like AInputQueue, InputConsumer). Implement a Dobby JNI hook targeting 'Unity_nativeInjectEvent' via 'libunity.so'. Extract X/Y coordinates from the Java MotionEvent, pass them to ImGui::GetIO().AddMousePosEvent(), and consume the touch if ImGui wants capture.
2. BUILD HEALING: Analyze the GitHub Actions workflow build failure error logs below. Fix ANY compilation, linking, CMake, Android.mk, or Gradle errors.
3. MAGISK PACKAGING: Ensure the build system correctly packages the compiled .so files alongside module.prop and customize.sh into a standard Magisk module zip format. Fix any zip packaging errors.

You MUST provide a short, descriptive git commit message summarizing your fix using this exact format:
=== COMMIT: [Your descriptive commit message here] ===

You MUST output the exact file modifications or deletions using these exact block formats:

To modify or create a file:
=== FILE: path/to/file ===
[File content here]
=== END FILE ===

To delete an obsolete file:
=== DELETE: path/to/file ===
=== END DELETE ===

ERROR LOGS:
{error_logs[-4000:]}
"""
    payload = {
        "contents": [{
            "parts": [{"text": prompt}]
        }]
    }

    for key_id, active_key in API_KEYS_POOL.items():
        if not active_key:
            continue
            
        print(f"\n[*] Switching to API Key ID: {key_id}")
        
        for model_name in MODELS_POOL:
            url = f"https://generativelanguage.googleapis.com/v1beta/{model_name}:generateContent?key={active_key}"
            print(f"[*] Trying Model: {model_name} with API Key ID: {key_id}...")

            try:
                response = requests.post(url, json=payload, timeout=30)
                res_json = response.json()
                if "candidates" in res_json:
                    print(f"[+] Success using Model: {model_name} on API Key ID: {key_id}")
                    return res_json["candidates"][0]["content"]["parts"][0]["text"]
                else:
                    error_msg = res_json.get('error', {}).get('message', 'Unknown Error')
                    print(f"[!] Model {model_name} failed with Key {key_id}: {error_msg}")
                    time.sleep(1)
            except Exception as e:
                print(f"[!] Network error for {model_name} with Key {key_id}: {str(e)}")
                time.sleep(1)

    return f"API Error / Limit Reached: All keys and JSON models failed completely."

def apply_ai_patches(ai_response):
    changes_made = []

    if not ai_response or "API Error" in ai_response:
        if os.path.exists("ai_fix_suggestion.txt"):
            with open("ai_fix_suggestion.txt", "r", encoding="utf-8") as f:
                ai_response = f.read()

    commit_match = re.search(r"=== COMMIT:\s*(.*?)\s*===", ai_response)
    commit_message = commit_match.group(1).strip() if commit_match else "fix: resolve build or touch implementation issue via AI"

    pattern_file = r"=== FILE:\s*(.*?)===\s*\n(.*?)\s*=== END FILE ==="
    matches_file = re.findall(pattern_file, ai_response, re.DOTALL)
    for file_path, content in matches_file:
        file_path = file_path.strip()
        dir_name = os.path.dirname(file_path)
        if dir_name and not os.path.exists(dir_name):
            os.makedirs(dir_name, exist_ok=True)
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(content.strip() + "\n")
        changes_made.append(f"Updated/Created: {file_path}")

    pattern_del = r"=== DELETE:\s*(.*?)===\s*=== END DELETE ==="
    matches_del = re.findall(pattern_del, ai_response, re.DOTALL)
    for file_path in matches_del:
        file_path = file_path.strip()
        if os.path.exists(file_path):
            os.remove(file_path)
            changes_made.append(f"Deleted: {file_path}")

    if not changes_made:
        with open("ai_fix_suggestion.txt", "w", encoding="utf-8") as f:
            f.write(ai_response)
        return [], commit_message

    return changes_made, commit_message

def master_loop():
    print("==================================================")
    print(" Starting Full-Auto Zygisk Builder & Touch Fixer")
    print("==================================================")

    last_processed_run_id = None

    while True:
        try:
            try:
                subprocess.run("termux-wake-lock", shell=True, capture_output=True)
            except Exception:
                pass

            print("[*] Checking GitHub for active workflow run...")
            run_id, status = get_latest_workflow_run()

            if not run_id or run_id == last_processed_run_id:
                time.sleep(15)
                continue

            print(f"[*] Monitoring Workflow Run ID: {run_id} | Status: {status}")

            while status in ["queued", "in_progress"] or status is None:
                time.sleep(15)
                _, status = get_latest_workflow_run()
                if status is None:
                    print("[!] Internet disconnected. Waiting for network...")
                else:
                    print(f"[*] Build is {status}... waiting for it to finish...")

            url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/actions/runs/{run_id}"

            try:
                run_details = requests.get(url, headers=HEADERS, timeout=15).json()
                conclusion = run_details.get("conclusion")
            except Exception as e:
                print(f"[!] Network error checking build conclusion: {e}. Retrying...")
                time.sleep(15)
                continue

            if conclusion == "success":
                print("\n==================================================")
                print(" SUCCESS! Zygisk Module Zip & Touch Fix compiled cleanly!")
                print("==================================================")
                last_processed_run_id = run_id
                print("[*] Waiting for new builds...\n")
                continue

            elif conclusion in ["failure", "cancelled", "timed_out"]:
                print(f"[!] Build failed with conclusion: {conclusion}. Initiating Auto-Heal...")

                logs = get_workflow_logs(run_id)

                if "Log fetch error" in logs:
                    print("[!] Skipping AI processing due to GitHub network error. Will retry in 30s...")
                    time.sleep(30)
                    continue

                print("[*] Analyzing module build/packaging errors with Gemini AI...")
                ai_fix = ask_gemini_http(logs)

                print("[*] Automatically applying AI patches to local files...")
                applied_changes, commit_message = apply_ai_patches(ai_fix)

                if applied_changes:
                    print(f"[+] CHANGES APPLIED: {', '.join(applied_changes)}")
                    print(f"[+] AI COMMIT MESSAGE: {commit_message}")
                    run_cmd("git add .")
                    safe_msg = commit_message.replace('"', '\\"')
                    run_cmd(f'git commit -m "{safe_msg}"')
                    run_cmd("git push origin main --force")
                    print("[+] Pushed code updates to GitHub!")

                    print("[+] Triggering a new workflow build to test the fixes...")
                    trigger_workflow_dispatch()
                    last_processed_run_id = run_id
                    time.sleep(20)
                else:
                    print("[!] No patch blocks found. Saved full response to ai_fix_suggestion.txt")
                    last_processed_run_id = run_id
                    time.sleep(15)

        except Exception as e:
            print(f"\n[CRITICAL ERROR] Script encountered an issue: {e}")
            print("[*] Don't worry, restarting loop in 15 seconds...\n")
            time.sleep(15)

if __name__ == "__main__":
    master_loop()
