#!/usr/bin/env python3

import os
import json
import uuid
from http import cookies
from datetime import datetime, timedelta

# Path to sessions/sessions.json (relative to this script)
BASE_DIR = os.path.dirname(__file__)
SESSIONS_FILE = os.path.join(BASE_DIR, "../sessions/sessions.json")

def load_sessions():
    path = os.path.abspath(SESSIONS_FILE)
    if os.path.exists(path):
        try:
            with open(path) as f:
                return json.load(f)
        except json.JSONDecodeError:
            return {}
    return {}

def save_sessions(sessions):
    path = os.path.abspath(SESSIONS_FILE)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        json.dump(sessions, f)

cookie = cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
sessions = load_sessions()

# Get or create session
if "session_id" in cookie:
    sid = cookie["session_id"].value
    
else:
    sid = str(uuid.uuid4())
    cookie["session_id"] = sid
    cookie["session_id"]["path"] = "/"
    cookie["session_id"]["max-age"] = 604800  # 7 days
    expires = (datetime.utcnow() + timedelta(days=7)).strftime("%a, %d %b %Y %H:%M:%S GMT")
    cookie["session_id"]["expires"] = expires

# Update session
if sid not in sessions:
    sessions[sid] = {"visits": 1}
else:
    sessions[sid]["visits"] += 1
save_sessions(sessions)

# Output
print("Content-Type: text/html")
print(cookie.output())
print()

# Load template file and substitute placeholders
template_path = os.path.join(BASE_DIR, 'session.html')
with open(template_path, 'r', encoding='utf-8') as f:
    template = f.read()

visits = sessions[sid]['visits']
visit_text = 'visit' if visits == 1 else 'visits'
body = template.replace('{sid}', sid).replace('{visits}', str(visits)).replace('{visit_text}', visit_text)

print(body)
