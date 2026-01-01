// Shared WiFi configuration functions
// Used by both captive.html and index.html

/**
 * Show status message
 * @param {string} elementId - ID of the status element
 * @param {string} message - Message to display
 * @param {string} type - Type: 'success', 'error', or 'info'
 */
function showStatus(elementId, message, type) {
    const status = document.getElementById(elementId);
    if (!status) return;

    status.textContent = message;
    status.className = `status ${type}`;

    if (type !== 'info') {
        setTimeout(() => {
            status.className = 'status';
        }, 5000);
    }
}

/**
 * Scan for available WiFi networks
 */
async function scanNetworks() {
    const loading = document.getElementById('loading');
    const networkList = document.getElementById('network-list');
    const select = document.getElementById('available-networks');

    // Show loading state
    if (loading) {
        loading.classList.add('active');
    }
    if (networkList) {
        networkList.style.display = 'none';
        networkList.innerHTML = '';
    }
    if (select) {
        select.innerHTML = `<option value="">${t('wifi.searching')}</option>`;
    }

    try {
        const response = await fetch('/api/wifi/scan');
        const networks = await response.json();

        if (loading) {
            loading.classList.remove('active');
        }

        // Sort by signal strength
        networks.sort((a, b) => b.rssi - a.rssi);

        // Render for captive portal (network list)
        if (networkList) {
            if (networks.length === 0) {
                networkList.innerHTML = `<div class="network-item"><span class="network-name">${t('devices.unpaired.empty')}</span></div>`;
            } else {
                networks.forEach(network => {
                    const signalIcon = getSignalIcon(network.rssi);
                    const item = document.createElement('div');
                    item.className = 'network-item';
                    item.onclick = () => selectNetwork(network.ssid, item);
                    item.innerHTML = `
                        <span class="network-name">
                            <span class="signal-icon">${signalIcon}</span>
                            ${escapeHtml(network.ssid)}
                        </span>
                        <span class="network-signal">${network.rssi} dBm</span>
                    `;
                    networkList.appendChild(item);
                });
            }
            networkList.style.display = 'block';
        }

        // Render for main interface (select dropdown)
        if (select) {
            select.innerHTML = `<option value="">${t('wifi.scan.hint')}</option>`;
            networks.forEach(network => {
                const option = document.createElement('option');
                option.value = network.ssid;
                option.textContent = `${network.ssid} (${network.rssi} dBm)`;
                select.appendChild(option);
            });
            // Note: onchange handler is set inline in HTML
        }

        showStatus('wifi-status', t('wifi.networks.found', { count: networks.length }), 'success');
    } catch (error) {
        if (loading) {
            loading.classList.remove('active');
        }

        // Demo mode for local testing
        const demoNetworks = [
            { ssid: 'Demo-Netzwerk', rssi: -45 },
            { ssid: 'Gast-WLAN', rssi: -67 },
            { ssid: 'Nachbar-WiFi', rssi: -82 }
        ];

        if (networkList) {
            demoNetworks.forEach(network => {
                const signalIcon = getSignalIcon(network.rssi);
                const item = document.createElement('div');
                item.className = 'network-item';
                item.onclick = () => selectNetwork(network.ssid, item);
                item.innerHTML = `
                    <span class="network-name">
                        <span class="signal-icon">${signalIcon}</span>
                        ${escapeHtml(network.ssid)}
                    </span>
                    <span class="network-signal">${network.rssi} dBm</span>
                `;
                networkList.appendChild(item);
            });
            networkList.style.display = 'block';
        }

        if (select) {
            select.innerHTML = `<option value="">${t('wifi.scan.hint')}</option>`;
            demoNetworks.forEach(network => {
                const option = document.createElement('option');
                option.value = network.ssid;
                option.textContent = `${network.ssid} (${network.rssi} dBm)`;
                select.appendChild(option);
            });
        }

        showStatus('wifi-status', 'Demo: ' + t('wifi.networks.found', { count: demoNetworks.length }), 'info');
    }
}

/**
 * Select a network from the list (captive portal)
 * @param {string} ssid - Network SSID
 * @param {HTMLElement} element - Clicked element
 */
function selectNetwork(ssid, element) {
    // Remove previous selection
    document.querySelectorAll('.network-item').forEach(item => {
        item.classList.remove('selected');
    });

    // Add selection to clicked item
    element.classList.add('selected');

    // Fill in SSID
    document.getElementById('ssid').value = ssid;

    // Focus password field
    document.getElementById('password').focus();
}

/**
 * Get signal strength icon
 * @param {number} rssi - Signal strength in dBm
 * @returns {string} Emoji icon
 */
function getSignalIcon(rssi) {
    if (rssi >= -50) return '📶';
    if (rssi >= -60) return '📶';
    if (rssi >= -70) return '📶';
    return '📶';
}

/**
 * Escape HTML to prevent XSS
 * @param {string} text - Text to escape
 * @returns {string} Escaped text
 */
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

/**
 * Save WiFi configuration
 */
async function saveWifi() {
    const ssid = document.getElementById('ssid').value.trim();
    const password = document.getElementById('password').value;

    if (!ssid) {
        showStatus('wifi-status', t('wifi.error.ssid'), 'error');
        return;
    }

    showStatus('wifi-status', t('common.loading'), 'info');

    try {
        const response = await fetch('/api/wifi/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ ssid, password })
        });

        if (response.ok) {
            showStatus('wifi-status', t('wifi.saved'), 'success');

            // Show countdown for captive portal
            if (document.querySelector('.info-box')) {
                let countdown = 10;
                const countdownInterval = setInterval(() => {
                    showStatus('wifi-status', t('captive.connecting', { seconds: countdown }), 'success');
                    countdown--;
                    if (countdown < 0) {
                        clearInterval(countdownInterval);
                        showStatus('wifi-status', t('captive.done'), 'success');
                    }
                }, 1000);
            }
        } else {
            const error = await response.json().catch(() => ({}));
            throw new Error(error.error || t('wifi.error.save'));
        }
    } catch (error) {
        if (error.message.includes('fetch')) {
            // Demo mode
            showStatus('wifi-status', 'Demo: ' + t('wifi.saved'), 'success');
        } else {
            showStatus('wifi-status', t('error') + ': ' + error.message, 'error');
        }
    }
}

/**
 * Update connection status (for main interface)
 */
async function updateConnectionStatus() {
    const connStatus = document.getElementById('conn-status');
    const connIp = document.getElementById('conn-ip');
    const connRssi = document.getElementById('conn-rssi');

    if (!connStatus) return;

    try {
        const response = await fetch('/api/wifi/status');
        const status = await response.json();

        connStatus.textContent = status.connected ? t('wifi.connected') : t('wifi.disconnected');
        if (connIp) connIp.textContent = status.ip || '-';
        if (connRssi) connRssi.textContent = status.rssi ? `${status.rssi} dBm` : '-';
    } catch (error) {
        connStatus.textContent = t('wifi.unavailable');
    }
}
