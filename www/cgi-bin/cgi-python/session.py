#!/usr/bin/env python3

import os
import json
import uuid
from http import cookies
from datetime import datetime, timedelta, timezone

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
    expires = (datetime.now(timezone.utc) + timedelta(days=7)).strftime("%a, %d %b %Y %H:%M:%S GMT")
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
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WebServ - Session Management</title>
    <style>
        /* Reset and base styles */
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            font-family: 'Inter', 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0a0a0a 0%, #1a1a1a 25%, #0f2a0f 50%, #1a1a1a 75%, #0a0a0a 100%);
            background-attachment: fixed;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            position: relative;
            overflow-x: hidden;
            color: #ffffff;
        }}

        /* Particle Background */
        .particles-background {{
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: 1;
            pointer-events: none;
        }}

        .particle {{
            position: absolute;
            background: rgba(0, 255, 0, 0.3);
            border-radius: 50%;
            animation: float 6s ease-in-out infinite;
            box-shadow: 0 0 10px rgba(0, 255, 0, 0.4);
        }}

        @keyframes float {{
            0%, 100% {{ transform: translateY(0) translateX(0); }}
            50% {{ transform: translateY(-20px) translateX(10px); }}
        }}

        /* Generate 15 particles with unique positions */
        .particle:nth-child(1) {{ width: 8px; height: 8px; top: 20%; left: 10%; animation-delay: 0s; }}
        .particle:nth-child(2) {{ width: 12px; height: 12px; top: 60%; left: 20%; animation-delay: 2s; }}
        .particle:nth-child(3) {{ width: 6px; height: 6px; top: 40%; left: 80%; animation-delay: 4s; }}
        .particle:nth-child(4) {{ width: 10px; height: 10px; top: 80%; left: 70%; animation-delay: 1s; }}
        .particle:nth-child(5) {{ width: 15px; height: 15px; top: 10%; left: 60%; animation-delay: 3s; }}
        .particle:nth-child(6) {{ width: 8px; height: 8px; top: 70%; left: 15%; animation-delay: 5s; }}
        .particle:nth-child(7) {{ width: 12px; height: 12px; top: 30%; left: 40%; animation-delay: 1.5s; }}
        .particle:nth-child(8) {{ width: 9px; height: 9px; top: 90%; left: 85%; animation-delay: 6s; }}
        .particle:nth-child(9) {{ width: 7px; height: 7px; top: 15%; left: 85%; animation-delay: 2.5s; }}
        .particle:nth-child(10) {{ width: 11px; height: 11px; top: 50%; left: 5%; animation-delay: 4.5s; }}
        .particle:nth-child(11) {{ width: 13px; height: 13px; top: 75%; left: 50%; animation-delay: 0.5s; }}
        .particle:nth-child(12) {{ width: 6px; height: 6px; top: 25%; left: 75%; animation-delay: 3.5s; }}
        .particle:nth-child(13) {{ width: 14px; height: 14px; top: 85%; left: 30%; animation-delay: 2.8s; }}
        .particle:nth-child(14) {{ width: 5px; height: 5px; top: 45%; left: 55%; animation-delay: 3.2s; }}
        .particle:nth-child(15) {{ width: 10px; height: 10px; top: 5%; left: 25%; animation-delay: 1.2s; }}

        /* Main container */
        .main-container {{
            width: 100%;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            position: relative;
            z-index: 2;
        }}

        /* Navigation */
        .navigation {{
            display: flex;
            justify-content: center;
            gap: 20px;
            margin-bottom: 40px;
            width: 100%;
        }}

        .nav-button {{
            padding: 10px 20px;
            background: rgba(0, 50, 0, 0.5);
            color: #00ff00;
            text-decoration: none;
            border-radius: 30px;
            font-weight: 600;
            letter-spacing: 1px;
            text-transform: uppercase;
            position: relative;
            overflow: hidden;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
            border: 1px solid rgba(0, 255, 0, 0.2);
        }}

        .nav-button:hover {{
            background: rgba(0, 60, 0, 0.7);
            transform: translateY(-3px);
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.3);
        }}

        /* Content */
        .content {{
            flex: 1;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            text-align: center;
            position: relative;
            background: rgba(0, 20, 0, 0.6);
            border-radius: 20px;
            padding: 40px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
            border: 1px solid rgba(0, 255, 0, 0.1);
            backdrop-filter: blur(10px);
            max-width: 800px;
            width: 100%;
        }}

        /* Session info */
        .session-title {{
            font-size: 3.5rem;
            font-weight: 800;
            color: #ffffff;
            margin-bottom: 30px;
            letter-spacing: 2px;
            text-shadow: 0 0 20px rgba(0, 255, 0, 0.5);
            animation: titleGlow 3s ease-in-out infinite;
            background: linear-gradient(45deg, #ffffff, #00ff00);
            background-clip: text;
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }}

        @keyframes titleGlow {{
            0%, 100% {{ text-shadow: 0 0 20px rgba(0, 255, 0, 0.5); }}
            50% {{ text-shadow: 0 0 30px rgba(0, 255, 0, 0.8), 0 0 40px rgba(0, 255, 0, 0.3); }}
        }}

        .session-info {{
            margin-bottom: 30px;
            padding: 20px;
            background: rgba(0, 30, 0, 0.6);
            border-radius: 15px;
            border: 1px solid rgba(0, 255, 0, 0.2);
        }}

        .session-id {{
            font-family: monospace;
            font-size: 1.2rem;
            color: #00ff00;
            margin-bottom: 15px;
            padding: 10px;
            background: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            display: inline-block;
            max-width: 100%;
            overflow-wrap: break-word;
        }}

        .visit-counter {{
            font-size: 2.5rem;
            font-weight: 700;
            color: #ffffff;
            margin: 20px 0;
            text-shadow: 0 0 10px rgba(0, 255, 0, 0.6);
        }}

        .visit-text {{
            color: #cccccc;
            font-size: 1.2rem;
            opacity: 0.9;
        }}

        .cookie-info {{
            margin-top: 30px;
            font-size: 0.9rem;
            color: #aaaaaa;
            max-width: 500px;
            line-height: 1.6;
        }}

        .back-link {{
            margin-top: 40px;
            color: #00ff00;
            text-decoration: none;
            padding: 12px 25px;
            background: rgba(0, 40, 0, 0.6);
            border-radius: 30px;
            font-weight: 600;
            transition: all 0.3s ease;
            border: 1px solid rgba(0, 255, 0, 0.2);
            display: inline-block;
        }}

        .back-link:hover {{
            background: rgba(0, 60, 0, 0.8);
            transform: translateY(-3px);
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.2);
        }}

        /* Pulse animation for visit counter */
        @keyframes pulse {{
            0% {{ transform: scale(1); }}
            50% {{ transform: scale(1.05); }}
            100% {{ transform: scale(1); }}
        }}

        .pulsate {{
            animation: pulse 2s infinite;
        }}
    </style>
</head>
<body>
    <div class="particles-background">
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
    </div>

    <div class="main-container">
        <nav class="navigation">
            <a href="/" class="nav-button">Home</a>
            <a href="/team/" class="nav-button">Team</a>
            <a href="/about/" class="nav-button">About</a>
            <a href="/tests/" class="nav-button">Tests</a>
        </nav>
        
        <div class="content">
            <h1 class="session-title">Session Management</h1>
            
            <div class="session-info">
                <div>Your Session ID:</div>
                <div class="session-id">{sid}</div>
                
                <div class="visit-counter pulsate">
                    {sessions[sid]['visits']}
                </div>
                <div class="visit-text">
                    {'visit' if sessions[sid]['visits'] == 1 else 'visits'} to this page
                </div>
            </div>
            
            <div class="cookie-info">
                <p>This demo shows how our WebServ handles cookies and session management. 
                A unique session ID is stored in your browser as a cookie, allowing the server to 
                remember your visits.</p>
            </div>
            
            <a href="/" class="back-link">Return to Homepage</a>
        </div>
    </div>
</body>
</html>
""")
