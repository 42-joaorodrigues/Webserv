#!/usr/bin/env python3

import requests
import time

def test_simple_upload(url, size_kb):
    """Test uploading a simple file of specified size in KB"""
    print(f"Testing upload of {size_kb}KB file...")
    
    # Generate simple content
    content = 'A' * (size_kb * 1024)
    
    # Prepare multipart form data
    files = {
        'file': (f'test_{size_kb}kb.txt', content, 'text/plain')
    }
    
    try:
        print(f"  Sending {len(content)} bytes...")
        response = requests.post(url, files=files, timeout=30)
        print(f"  Status Code: {response.status_code}")
        if response.status_code != 200:
            print(f"  Response: {response.text[:200]}...")
        return response.status_code == 200
    except requests.exceptions.RequestException as e:
        print(f"  Error: {e}")
        return False

if __name__ == "__main__":
    upload_url = "http://127.0.0.1:8080/upload"
    
    # Test progressively larger sizes to find the breaking point
    test_sizes = [10, 50, 100, 200, 300, 400, 500, 1000]  # KB
    
    print("Testing to find the breaking point...")
    print("=" * 50)
    
    for size in test_sizes:
        success = test_simple_upload(upload_url, size)
        print(f"  Result: {'SUCCESS' if success else 'FAILED'}")
        print("-" * 30)
        
        if not success:
            print(f"Breaking point found around {size}KB")
            break
        
        # Small delay between tests
        time.sleep(0.5)