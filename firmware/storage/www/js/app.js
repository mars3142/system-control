// Global variables
let schemaData = [];
let currentEditRow = null;
let lightOn = false;
let currentMode = 'simulation';
let ws = null;
let wsReconnectTimer = null;
let pairedDevices = [];
let scenes = [];
let currentEditScene = null;
let selectedSceneIcon = '🌅';

// Event listeners
document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') {
        closeColorModal();
        closeSceneModal();
    }
});

document.getElementById('color-modal').addEventListener('click', (e) => {
    if (e.target.classList.contains('modal-overlay')) {
        closeColorModal();
    }
});

document.getElementById('scene-modal').addEventListener('click', (e) => {
    if (e.target.classList.contains('modal-overlay')) {
        closeSceneModal();
    }
});

// Prevent zoom on double-tap for iOS
let lastTouchEnd = 0;
document.addEventListener('touchend', (e) => {
    const now = Date.now();
    if (now - lastTouchEnd <= 300) {
        e.preventDefault();
    }
    lastTouchEnd = now;
}, false);

document.addEventListener('DOMContentLoaded', async () => {
    initI18n();
    initTheme();
    await initCapabilities();
    initWebSocket();
    updateConnectionStatus();
    loadLightStatus();

    // Only load scenes and devices if thread is enabled
    if (isThreadEnabled()) {
        loadScenes();
        loadPairedDevices();
    }
    // WiFi status polling (less frequent)
    setInterval(updateConnectionStatus, 30000);
});

// Close WebSocket on page unload
window.addEventListener('beforeunload', () => {
    if (ws) {
        ws.close();
    }
});
