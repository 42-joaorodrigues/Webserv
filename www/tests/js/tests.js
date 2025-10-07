// Test Statistics
let testStats = {
    total: 0,
    successful: 0,
    responseTimes: []
};

// Matrix login state
let matrixUser = {
    username: null,
    isLoggedIn: false
};

// File for upload
let selectedFile = null;

// Utility function to make HTTP requests
async function makeRequest(method, url, data = null, isFile = false) {
    const startTime = Date.now();
    
    try {
        const options = {
            method: method,
            headers: {}
        };

        if (data && !isFile) {
            if (method === 'POST' && typeof data === 'string') {
                options.headers['Content-Type'] = 'application/x-www-form-urlencoded';
            } else if (typeof data === 'object') {
                options.headers['Content-Type'] = 'application/json';
                data = JSON.stringify(data);
            }
            options.body = data;
        } else if (isFile && data instanceof FormData) {
            options.body = data;
        }

        const response = await fetch(url, options);
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        // Update stats
        testStats.total++;
        testStats.responseTimes.push(responseTime);
        
        if (response.ok) {
            testStats.successful++;
        }
        
        updateStats();
        
        let responseText = '';
        const contentType = response.headers.get('content-type');
        
        if (contentType && contentType.includes('application/json')) {
            try {
                const jsonData = await response.json();
                responseText = JSON.stringify(jsonData, null, 2);
            } catch (e) {
                responseText = await response.text();
            }
        } else {
            responseText = await response.text();
        }
        
        return {
            ok: response.ok,
            status: response.status,
            statusText: response.statusText,
            headers: Object.fromEntries(response.headers.entries()),
            body: responseText,
            responseTime: responseTime
        };
    } catch (error) {
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        testStats.total++;
        testStats.responseTimes.push(responseTime);
        updateStats();
        
        return {
            ok: false,
            status: 0,
            statusText: 'Network Error',
            headers: {},
            body: error.message,
            responseTime: responseTime,
            error: true
        };
    }
}

// Display test results
function displayResult(elementId, result) {
    const element = document.getElementById(elementId);
    element.innerHTML = '';
    element.className = 'test-result';
    
    if (result.error) {
        element.classList.add('result-error');
    } else if (result.ok) {
        element.classList.add('result-success');
    } else {
        element.classList.add('result-error');
    }
    
    let output = `Status: ${result.status} ${result.statusText}\n`;
    output += `Response Time: ${result.responseTime}ms\n`;
    output += `Headers:\n${JSON.stringify(result.headers, null, 2)}\n`;
    output += `\nBody:\n${result.body}`;
    
    element.textContent = output;
}

// Update statistics display
function updateStats() {
    document.getElementById('totalTests').textContent = testStats.total;
    document.getElementById('successfulTests').textContent = testStats.successful;
    
    if (testStats.responseTimes.length > 0) {
        const avgTime = testStats.responseTimes.reduce((a, b) => a + b, 0) / testStats.responseTimes.length;
        document.getElementById('avgResponseTime').textContent = Math.round(avgTime) + 'ms';
    }
}

// Test Functions

// GET Request Test
async function testGet() {
    const url = document.getElementById('getUrl').value || '/';
    const resultElement = document.getElementById('getResult');
    
    resultElement.innerHTML = 'Testing GET request...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('GET', url);
    displayResult('getResult', result);
}

// POST Request Test
async function testPost() {
    const url = document.getElementById('postUrl').value || '/';
    const data = document.getElementById('postData').value;
    const resultElement = document.getElementById('postResult');
    
    resultElement.innerHTML = 'Testing POST request...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('POST', url, data);
    displayResult('postResult', result);
}

// DELETE Request Test
async function testDelete() {
    const url = document.getElementById('deleteUrl').value || '/';
    const resultElement = document.getElementById('deleteResult');
    
    resultElement.innerHTML = 'Testing DELETE request...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('DELETE', url);
    displayResult('deleteResult', result);
}

// File Upload Test
async function testUpload() {
    if (!selectedFile) {
        alert('Please select a file first');
        return;
    }
    
    const uploadPath = document.getElementById('uploadPath').value || '/files';
    const resultElement = document.getElementById('uploadResult');
    
    resultElement.innerHTML = 'Uploading file...';
    resultElement.className = 'test-result loading';
    
    const formData = new FormData();
    formData.append('file', selectedFile);
    
    const result = await makeRequest('POST', uploadPath, formData, true);
    displayResult('uploadResult', result);
}

// Python CGI Test
async function testPythonCgi() {
    const url = document.getElementById('pythonCgiUrl').value || '/cgi-python/test.py';
    const data = document.getElementById('pythonCgiData').value;
    const resultElement = document.getElementById('pythonCgiResult');
    
    resultElement.innerHTML = 'Testing Python CGI...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('POST', url, data);
    displayResult('pythonCgiResult', result);
}

// Bash CGI Test
async function testBashCgi() {
    const url = document.getElementById('bashCgiUrl').value || '/cgi-bash/test.sh';
    const data = document.getElementById('bashCgiData').value;
    const resultElement = document.getElementById('bashCgiResult');
    
    resultElement.innerHTML = 'Testing Bash CGI...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('POST', url, data);
    displayResult('bashCgiResult', result);
}

// Directory Listing Test
async function testDirectory() {
    const url = document.getElementById('dirUrl').value || '/files/';
    const resultElement = document.getElementById('dirResult');
    
    resultElement.innerHTML = 'Testing directory listing...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('GET', url);
    displayResult('dirResult', result);
}

// Autoindex Test
async function testAutoindex() {
    const url = document.getElementById('autoindexUrl').value || '/files/';
    const resultElement = document.getElementById('autoindexResult');
    
    resultElement.innerHTML = 'Testing autoindex...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('GET', url);
    displayResult('autoindexResult', result);
}

// Error Tests
async function test404() {
    const resultElement = document.getElementById('error404Result');
    
    resultElement.innerHTML = 'Testing 404 error...';
    resultElement.className = 'test-result loading';
    
    const result = await makeRequest('GET', '/nonexistent-file-' + Date.now());
    displayResult('error404Result', result);
}

async function test403() {
    const resultElement = document.getElementById('error403Result');
    
    resultElement.innerHTML = 'Testing 403 error...';
    resultElement.className = 'test-result loading';
    
    try {
        await makeRequest('GET', '/files/forbidden.txt');
        resultElement.innerHTML = '<div class="result-error">Test failed: Expected 403 error but got success</div>';
        resultElement.className = 'test-result failed';
    } catch (error) {
        if (error.status === 403) {
            resultElement.innerHTML = '<div class="result-success">Test passed: Got 403 Forbidden as expected</div>';
            resultElement.className = 'test-result success';
        } else {
            resultElement.innerHTML = `<div class="result-error">Test failed: Expected 403 error but got ${error.status || 'unknown error'}</div>`;
            resultElement.className = 'test-result failed';
        }
    }
}

function confirmCookie() {
    const warning = document.createElement('div');
    warning.innerHTML = "⚠️ Are you *sure* you want to take the cookie? This might change... everything 🍪";
    warning.style.color = 'red';
    warning.style.fontWeight = 'bold';
    warning.style.padding = '10px';
    warning.style.background = '#2a0000';
    warning.style.border = '2px solid red';
    warning.style.borderRadius = '10px';
    warning.style.textAlign = 'center';
    warning.style.marginTop = '10px';
    warning.style.animation = 'flash 0.5s infinite alternate';

    const container = document.getElementById('CookieTest');
    container.innerHTML = '';
    container.appendChild(warning);

    // small dramatic pause
    setTimeout(() => {
        if (confirm("😈 Are you *really* sure you want the cookie?")) {
            window.location.href = 'http://127.0.0.1:8080/cgi-python/session.py';
        } else {
            container.innerHTML = "<span style='color:green;'>Wise choice... for now 🍀</span>";
        }
    }, 1500);
}

async function test413() {
    console.log('=== test413 FUNCTION CALLED ===');
    const resultElement = document.getElementById('error413Result');
    
    resultElement.innerHTML = 'Testing 413 error...';
    resultElement.className = 'test-result loading';
    
    try {
        console.log('Creating 3.1MB file...');
        
        // Create a simple text file that's 2MB
        const largeContent = 'A'.repeat(2 * 1024 * 1024); // 3.1MB of 'A' characters
        console.log('Content created, length:', largeContent.length, 'bytes');
        
        const blob = new Blob([largeContent], { type: 'text/plain' });
        const file = new File([blob], 'test-413.txt', { type: 'text/plain' });
        console.log('File object created, name:', file.name, 'size:', file.size, 'bytes');
        
        // Create FormData for file upload
        const formData = new FormData();
        formData.append('file', file);
        console.log('FormData created with file attached');
        
        // Try to upload the 2MB file to /files
        console.log('Attempting to upload 3.1MB file to /files...');
        const result = await makeRequest('POST', '/upload', formData, true);
        console.log('Upload result:', result.status, result.statusText);
        
        displayResult('error413Result', result);
        console.log('=== test413 FUNCTION COMPLETED SUCCESSFULLY ===');
    } catch (error) {
        console.error('=== ERROR CAUGHT IN test413 ===', error);
        displayResult('error413Result', {
            status: 0,
            statusText: 'Error',
            error: true,
            body: error.message,
            headers: {},
            responseTime: 0
        });
    }
}

// File Selection Handler
function handleFileSelect(event) {
    const file = event.target.files[0];
    if (file) {
        selectedFile = file;
        const uploadArea = document.querySelector('.upload-area');
        const uploadText = document.querySelector('.upload-text');
        uploadText.textContent = `Selected: ${file.name} (${formatFileSize(file.size)})`;
        uploadArea.style.borderColor = '#4CAF50';
        uploadArea.style.background = 'rgba(76, 175, 80, 0.1)';
    }
}

// Format file size
function formatFileSize(bytes) {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}