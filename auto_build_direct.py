import urllib.request
import urllib.error
import json
import os
import sys

# 1. Grab the API key you exported in Termux
api_key = os.environ.get("GEMINI_API_KEY")
if not api_key:
    print("Error: GEMINI_API_KEY is not set in your terminal.")
    sys.exit(1)

# 2. Define the files
input_file = "module/src/main/cpp/main.cpp" # Change to your actual C++ file path
output_file = "module/src/main/cpp/main_fixed.cpp"

print(f"Reading target file: {input_file}...")
try:
    with open(input_file, "r") as f:
        cpp_code = f.read()
except FileNotFoundError:
    print(f"File not found: {input_file}. Please check the path.")
    sys.exit(1)

# 3. Formulate the strict prompt for the AI
prompt = f"""
You are a C++ Zygisk developer.
Here is my current Zygisk ImGui source code. It currently uses standard Android native input hooks.

I need you to:
1. Remove all AInputQueue/InputConsumer touch logic.
2. Replace it with a Dobby JNI hook targeting 'Unity_nativeInjectEvent' via 'libunity.so'.
3. The hook must extract X/Y coordinates from the Java MotionEvent and pass them to ImGui::GetIO().AddMousePosEvent().
4. If ImGui WantsCaptureMouse, consume the touch. Otherwise, pass it to orig_nativeInjectEvent.
5. Output ONLY the raw C++ code. No markdown formatting, no explanations.

Here is the source code to patch:
{cpp_code}
"""

# 4. Build the direct HTTPS POST request to Google AI Studio
url = f"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key={api_key}"
headers = {'Content-Type': 'application/json'}
payload = {
    "contents": [{"parts": [{"text": prompt}]}]
}

data = json.dumps(payload).encode('utf-8')
req = urllib.request.Request(url, data=data, headers=headers, method='POST')

print("Making direct HTTPS request to Google AI Studio...")

# 5. Send the request and parse the response
try:
    with urllib.request.urlopen(req) as response:
        response_data = json.loads(response.read().decode('utf-8'))
        
        # Extract the text from the JSON response
        fixed_code = response_data['candidates'][0]['content']['parts'][0]['text'].strip()
        
        # Strip markdown code blocks if the AI accidentally includes them
        if fixed_code.startswith("```cpp"):
            fixed_code = fixed_code[6:]
        if fixed_code.endswith("```"):
            fixed_code = fixed_code[:-3]

        # 6. Save the new fixed C++ file
        with open(output_file, "w") as f:
            f.write(fixed_code.strip())
            
        print(f"SUCCESS: Touch fix applied! Saved to {output_file}")

except urllib.error.HTTPError as e:
    print(f"HTTP Error: {e.code} - {e.read().decode()}")
except Exception as e:
    print(f"Failed to generate code: {e}")

