// Capabilities Module
// Checks device capabilities and controls feature visibility

let capabilities = {
    thread: false
};

/**
 * Initialize capabilities module
 * Fetches from server, falls back to URL parameter for offline testing
 */
async function initCapabilities() {
    // Try to fetch from server first
    const success = await fetchCapabilities();

    // If server not available, check URL parameter (for offline testing)
    if (!success) {
        const urlParams = new URLSearchParams(window.location.search);
        if (urlParams.get('thread') === 'true') {
            capabilities.thread = true;
        }
    }

    // Apply visibility based on capabilities
    applyCapabilities();
}

/**
 * Fetch capabilities from server
 * @returns {boolean} true if successful
 */
async function fetchCapabilities() {
    try {
        const response = await fetch('/api/capabilities');
        if (response.ok) {
            const data = await response.json();
            capabilities = { ...capabilities, ...data };
            return true;
        }
        return false;
    } catch (error) {
        console.log('Capabilities not available, using defaults');
        return false;
    }
}

/**
 * Check if thread/Matter is enabled
 * @returns {boolean}
 */
function isThreadEnabled() {
    return capabilities.thread === true;
}

/**
 * Apply capabilities to UI - show/hide elements
 */
function applyCapabilities() {
    const threadEnabled = isThreadEnabled();

    // Elements to show/hide based on thread capability
    const threadElements = [
        // Control tab elements
        'scenes-control-card',
        'devices-control-card',
        // Config sub-tabs
        'subtab-btn-devices',
        'subtab-btn-scenes',
        // Config sub-tab contents
        'subtab-devices',
        'subtab-scenes'
    ];

    threadElements.forEach(id => {
        const element = document.getElementById(id);
        if (element) {
            element.style.display = threadEnabled ? '' : 'none';
        }
    });

    // Also hide scene devices section in scene modal if thread disabled
    const sceneDevicesSection = document.querySelector('#scene-modal .form-group:has(#scene-devices-list)');
    if (sceneDevicesSection) {
        sceneDevicesSection.style.display = threadEnabled ? '' : 'none';
    }
}

/**
 * Get all capabilities
 * @returns {object}
 */
function getCapabilities() {
    return { ...capabilities };
}
