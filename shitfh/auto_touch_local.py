import os
import time
import subprocess
import requests
import re

# ==========================================
# LOCAL LLAMA SERVER CONFIGURATION (Qwen)
# ==========================================
LOCAL_API_URL = "http://127.0.0.1:8080/v1/chat/completions"

print("==================================================")
print(" Starting Local AI Master Healer (Qwen Mode)")
print("==================================================")
_owner = input("Enter GitHub Username/Owner (Press Enter for 'jason8105'): ").strip()
REPO_OWNER = _owner if _owner else "jason8105"

_repo = input("Enter Repository Name (Press Enter for 'Zygisk-imgui-touch-fix'): ").strip()
REPO_NAME = _repo if _repo else "Zygisk-imgui-touch-fix"

print(f"\n[*] Target Repository set to: {REPO_OWNER}/{REPO_NAME}")
print("==================================================\n")

GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")

HEADERS = {
    "Authorization": f"Bearer {GITHUB_TOKEN}",
    "Accept": "vnd.github+json"
} if GITHUB_TOKEN else {}

def run_cmd(cmd):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.stdout + result.stderr

def trigger_workflow_dispatch():
    url = "https://api.github.com/repos/" + REPO_OWNER + "/" + REPO_NAME + "/actions/workflows/build.yml/dispatches"
    try:
        requests.post(url, headers=HEADERS, json={"ref": "main"}, timeout=10)
    except Exception:
        pass

def get_latest_workflow_run():
    url = "https://api.github.com/repos/" + REPO_OWNER + "/" + REPO_NAME + "/actions/runs?per_page=1"
    try:
        res = requests.get(url, headers=HEADERS, timeout=15).json()
        runs = res.get("workflow_runs", [])
        if not runs:
            return None, None
        return runs[0]["id"], runs[0]["status"]
    except Exception:
        return None, None

def check_artifact_size(run_id):
    url = "https://api.github.com/repos/" + REPO_OWNER + "/" + REPO_NAME + "/actions/runs/" + str(run_id) + "/artifacts"
    try:
        res = requests.get(url, headers=HEADERS, timeout=15).json()
        artifacts = res.get("artifacts", [])
        if not artifacts:
            print("[!] Warning: No build artifacts found in this run!")
            return False
            
        for art in artifacts:
            size = art.get("size_in_bytes", 0)
            name = art.get("name", "unknown")
            print(f"[*] Found Artifact: '{name}' | Size: {size} bytes")
            if size < 5000:
                print(f"[!] Critical: Artifact size is too small ({size} bytes). Missing compiled binaries (.so files)!")
                return False
        return True
    except Exception as e:
        print(f"[!] Error verifying artifact size: {e}")
        return True

def get_workflow_logs(run_id, max_retries=5):
    print("[*] Fetching failed job details to get direct text logs...")
    jobs_url = "https://api.github.com/repos/" + REPO_OWNER + "/" + REPO_NAME + "/actions/runs/" + str(run_id) + "/jobs"

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
                    log_url = "https://api.github.com/repos/" + REPO_OWNER + "/" + REPO_NAME + "/actions/jobs/" + str(job_id) + "/logs"

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

def ask_local_ai(error_logs):
    prompt = f"""
You are an elite Android NDK, C++, Gradle, and Zygisk module build engineer.
Your mission is to ensure this repository compiles into a fully functional, flashable Magisk Zygisk module zip that contains a UNIVERSAL touch-fixed ImGui menu (supporting all game engines like Unity, Unreal, and native C++).

CRITICAL REQUIREMENTS:
1. UNIVERSAL TOUCH FIX: Implement a robust, universal input hook (such as standard Android AInputQueue, InputConsumer, or native event dispatches) that works across all game engines. Extract touch X/Y coordinates, pass them to ImGui::GetIO().AddMousePosEvent(), and consume the touch if ImGui wants mouse capture. Do not restrict it to a single engine like Unity.
2. BUILD HEALING & PACKAGING FIX (IMPORTANT): Ensure that the compiled native shared library (`libzygisk.so`) is successfully packed into the final Magisk module zip structure under `zygisk/<abi>/libzygisk.so`. Fix any Gradle or CMake output paths so the final zip file size is correct and contains all compiled binaries (avoiding empty 700-byte text-only zips).
3. MAGISK 24-26 COMPATIBILITY: Target Magisk versions 24 through 26 exclusively. Ensure `module.prop` sets `minMagisk` to `24000` and uses stable Zygisk entry points native to Magisk v24–26 without breaking changes.

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

ERROR LOGS / STATUS CONTEXT:
{error_logs[-4000:]}
"""
    payload = {
        "model": "qwen2.5-coder-7b-instruct",
        "messages": [
            {"role": "system", "content": "You are a precise code-fixing machine. Output ONLY the file blocks using === FILE: path === and === END FILE ===. Do not write explanations."},
            {"role": "user", "content": prompt}
        ],
        "temperature": 0.1,
        "max_tokens": 4096
    }

    print("[*] Asking Local AI (llama-server)...")
    try:
        response = requests.post(LOCAL_API_URL, json=payload, timeout=120)
        res_json = response.json()
        if "choices" in res_json:
            print("[+] Success getting response from Local AI!")
            return res_json["choices"][0]["message"]["content"]
        else:
            return f"Local API Error: {res_json}"
    except Exception as e:
        return f"Local Server Connection Failed: {str(e)}"

def apply_ai_patches(ai_response):
    changes_made = []

    if not ai_response or "Local API Error" in ai_response or "Local Server Connection Failed" in ai_response:
        if os.path.exists("ai_fix_suggestion.txt"):
            with open("ai_fix_suggestion.txt", "r", encoding="utf-8") as f:
                ai_response = f.read()

    ai_response = re.sub(r"^```[a-zA-Z]*\n", "", ai_response, flags=re.MULTILINE)

    commit_match = re.search(r"=== COMMIT:\s*([^\n]+)\s*===", ai_response)
    commit_message = commit_match.group(1).strip() if commit_match else "fix: resolve packaging path and ensure compiled binaries are included in zip"

    pattern_file = r"=== FILE:\s*([^\n]+)===\s*\n(.*?)(?==== FILE:|=== DELETE:|\Z)"
    matches_file = re.findall(pattern_file, ai_response, re.DOTALL)
    
    pattern_explicit = r"=== FILE:\s*([^\n]+)===\s*\n(.*?)\s*=== END FILE ==="
    matches_explicit = re.findall(pattern_explicit, ai_response, re.DOTALL)
    if matches_explicit:
        matches_file = matches_explicit

    for file_path, content in matches_file:
        file_path = file_path.strip().replace("\r", "").strip("`").strip()
        content = content.strip().replace("=== END FILE ===", "").strip()
        
        if content.startswith("```"):
            content = re.sub(r"^```[a-zA-Z]*\n", "", content)
        if content.endswith("```"):
            content = re.sub(r"\n```\s*$", "", content)
            
        content = content.strip()

        if not file_path or len(file_path) > 200:
            continue
            
        dir_name = os.path.dirname(file_path)
        if dir_name and not os.path.exists(dir_name):
            os.makedirs(dir_name, exist_ok=True)
            
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(content + "\n")
        changes_made.append(f"Updated/Created: {file_path}")

    pattern_del = r"=== DELETE:\s*([^\n]+)===\s*(?:=== END DELETE ===)?"
    matches_del = re.findall(pattern_del, ai_response, re.DOTALL)
    for file_path in matches_del:
        file_path = file_path.strip().replace("\r", "").strip("`").strip()
        if file_path and os.path.exists(file_path):
            os.remove(file_path)
            changes_made.append(f"Deleted: {file_path}")

    if not changes_made:
        print("[!] Warning: Response received from model but could not parse patch blocks. Saving to ai_fix_suggestion.txt")
        with open("ai_fix_suggestion.txt", "w", encoding="utf-8") as f:
            f.write(ai_response)
        return [], commit_message

    return changes_made, commit_message

def master_loop():
    print("==================================================")
    print(" Starting Local AI Master Healer (Qwen Mode)")
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

            url = "[https://api.github.com/repos/](https://api.github.com/repos/)" + REPO_OWNER + "/" + REPO_NAME + "/actions/runs/" + str(run_id)

            try:
                run_details = requests.get(url, headers=HEADERS, timeout=15).json()
                conclusion = run_details.get("conclusion")
            except Exception as e:
                print(f"[!] Network error checking build conclusion: {e}. Retrying...")
                time.sleep(15)
                continue

            if conclusion == "success":
                print("[*] GitHub Actions reports build success. Checking artifact contents & size...")
                if check_artifact_size(run_id):
                    print("\n==================================================")
                    print(" SUCCESS! Valid Zygisk Module Zip compiled cleanly!")
                    print("==================================================")
                    last_processed_run_id = run_id
                    print("[*] Waiting for new builds...\n")
                    continue
                else:
                    print("[!] Artifact validation failed (empty zip size). Forcing Auto-Heal...")
                    conclusion = "failure"

            if conclusion in ["failure", "cancelled", "timed_out"]:
                print(f"[!] Build failed or artifact was invalid (conclusion: {conclusion}). Initiating Auto-Heal...")

                logs = get_workflow_logs(run_id)
                if "Log fetch error" in logs:
                    logs = "Build artifact was empty/invalid (~746 bytes). Gradle failed to package libzygisk.so into the zip structure."

                print("[*] Analyzing errors with Local Qwen AI...")
                ai_fix = ask_local_ai(logs)

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
