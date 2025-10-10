#!/usr/bin/env python3

import requests
import threading
import time
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

# Test configuration
SERVER_URL = "http://localhost:8081/"
NUM_WORKERS = 20
REQUESTS_PER_WORKER = 5000  # Match ubuntu_tester: 100,000 total
TIMEOUT = 5  # seconds

class StressTest:
    def __init__(self):
        self.success_count = 0
        self.error_count = 0
        self.errors = []
        self.lock = threading.Lock()
    
    def single_request(self, worker_id, request_id):
        """Perform a single GET request"""
        try:
            response = requests.get(SERVER_URL, timeout=TIMEOUT)
            if response.status_code == 200:
                with self.lock:
                    self.success_count += 1
                    if (self.success_count + self.error_count) % 1000 == 0:
                        print(f"Progress: {self.success_count + self.error_count} requests completed")
                return True
            else:
                with self.lock:
                    self.error_count += 1
                    self.errors.append(f"Worker {worker_id}, Request {request_id}: HTTP {response.status_code}")
                return False
        except Exception as e:
            with self.lock:
                self.error_count += 1
                error_msg = f"Worker {worker_id}, Request {request_id}: {type(e).__name__}: {e}"
                self.errors.append(error_msg)
                print(f"ERROR: {error_msg}")
            return False
    
    def worker_thread(self, worker_id):
        """Worker thread that makes multiple requests"""
        print(f"Worker {worker_id} starting {REQUESTS_PER_WORKER} requests...")
        
        for request_id in range(REQUESTS_PER_WORKER):
            success = self.single_request(worker_id, request_id)
            
            # Small delay to prevent overwhelming the server initially
            time.sleep(0.001)  # 1ms between requests per worker
            
        print(f"Worker {worker_id} completed")
    
    def run_test(self):
        """Run the stress test"""
        print(f"Starting stress test: {NUM_WORKERS} workers × {REQUESTS_PER_WORKER} requests = {NUM_WORKERS * REQUESTS_PER_WORKER} total")
        print(f"Target server: {SERVER_URL}")
        
        start_time = time.time()
        
        # Use ThreadPoolExecutor to manage worker threads
        with ThreadPoolExecutor(max_workers=NUM_WORKERS) as executor:
            # Submit all worker tasks
            futures = [executor.submit(self.worker_thread, worker_id) for worker_id in range(NUM_WORKERS)]
            
            # Wait for all workers to complete
            for future in as_completed(futures):
                try:
                    future.result()  # Get result or exception
                except Exception as e:
                    print(f"Worker thread failed: {e}")
        
        end_time = time.time()
        duration = end_time - start_time
        total_requests = NUM_WORKERS * REQUESTS_PER_WORKER
        
        print(f"\n=== STRESS TEST RESULTS ===")
        print(f"Duration: {duration:.2f} seconds")
        print(f"Total requests: {total_requests}")
        print(f"Successful: {self.success_count}")
        print(f"Failed: {self.error_count}")
        print(f"Success rate: {(self.success_count / total_requests * 100):.1f}%")
        print(f"Requests/second: {total_requests / duration:.1f}")
        
        if self.errors:
            print(f"\nFirst 10 errors:")
            for error in self.errors[:10]:
                print(f"  {error}")
            
            if len(self.errors) > 10:
                print(f"  ... and {len(self.errors) - 10} more errors")
        
        # Return success if no errors
        return self.error_count == 0

if __name__ == "__main__":
    # Test basic connectivity first
    try:
        response = requests.get(SERVER_URL, timeout=5)
        print(f"Server connectivity test: HTTP {response.status_code}")
        print(f"Response: {response.text[:100]}...")
    except Exception as e:
        print(f"ERROR: Cannot connect to server: {e}")
        sys.exit(1)
    
    # Run the stress test
    test = StressTest()
    success = test.run_test()
    
    sys.exit(0 if success else 1)