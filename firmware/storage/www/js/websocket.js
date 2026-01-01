// WebSocket connection
function initWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;

    try {
        ws = new WebSocket(wsUrl);

        ws.onopen = () => {
            console.log('WebSocket connected');
            clearTimeout(wsReconnectTimer);
            // Request initial status
            ws.send(JSON.stringify({ type: 'getStatus' }));
        };

        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                handleWebSocketMessage(data);
            } catch (e) {
                console.error('WebSocket message error:', e);
            }
        };

        ws.onclose = () => {
            console.log('WebSocket disconnected, reconnecting in 3s...');
            ws = null;
            wsReconnectTimer = setTimeout(initWebSocket, 3000);
        };

        ws.onerror = (error) => {
            console.error('WebSocket error:', error);
            ws.close();
        };
    } catch (error) {
        console.log('WebSocket not available, using demo mode');
        initDemoMode();
    }
}

function handleWebSocketMessage(data) {
    switch (data.type) {
        case 'status':
            updateStatusFromData(data);
            break;
        case 'color':
            updateColorPreview(data.r, data.g, data.b);
            break;
        case 'wifi':
            updateWifiStatus(data);
            break;
    }
}

function updateStatusFromData(status) {
    if (status.on !== undefined) {
        lightOn = status.on;
        updateLightToggle();
    }

    if (status.mode) {
        currentMode = status.mode;
        updateModeButtons();
        updateSimulationOptions();
    }

    if (status.schema) {
        document.getElementById('active-schema').value = status.schema;
        const schemaNames = {
            'schema_01.csv': 'Schema 1',
            'schema_02.csv': 'Schema 2',
            'schema_03.csv': 'Schema 3'
        };
        document.getElementById('current-schema').textContent = schemaNames[status.schema] || status.schema;
    }

    if (status.color) {
        updateColorPreview(status.color.r, status.color.g, status.color.b);
    }
}

function updateColorPreview(r, g, b) {
    const colorPreview = document.getElementById('current-color');
    colorPreview.style.background = `rgb(${r}, ${g}, ${b})`;
}

function updateWifiStatus(status) {
    document.getElementById('conn-status').textContent = status.connected ? '✅ Verbunden' : '❌ Nicht verbunden';
    document.getElementById('conn-ip').textContent = status.ip || '-';
    document.getElementById('conn-rssi').textContent = status.rssi ? `${status.rssi} dBm` : '-';
}

// Send via WebSocket
function wsSend(data) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(data));
        return true;
    }
    return false;
}

// Demo mode for local testing
function initDemoMode() {
    updateSimulationOptions();
    updateColorPreview(255, 240, 220);

    // Simulate color changes in demo mode
    let hue = 0;
    setInterval(() => {
        if (!ws) {
            hue = (hue + 1) % 360;
            const rgb = hslToRgb(hue / 360, 0.7, 0.6);
            updateColorPreview(rgb.r, rgb.g, rgb.b);
        }
    }, 100);
}

function hslToRgb(h, s, l) {
    let r, g, b;
    if (s === 0) {
        r = g = b = l;
    } else {
        const hue2rgb = (p, q, t) => {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1 / 6) return p + (q - p) * 6 * t;
            if (t < 1 / 2) return q;
            if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
            return p;
        };
        const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        const p = 2 * l - q;
        r = hue2rgb(p, q, h + 1 / 3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1 / 3);
    }
    return { r: Math.round(r * 255), g: Math.round(g * 255), b: Math.round(b * 255) };
}
