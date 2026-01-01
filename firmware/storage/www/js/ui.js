// Theme management
function initTheme() {
    const savedTheme = localStorage.getItem('theme') || 'dark';
    setTheme(savedTheme);
}

function setTheme(theme) {
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);

    const icon = document.getElementById('theme-icon');
    const label = document.getElementById('theme-label');
    const metaTheme = document.querySelector('meta[name="theme-color"]');

    if (theme === 'light') {
        icon.textContent = '☀️';
        label.textContent = 'Light';
        metaTheme.content = '#f0f2f5';
    } else {
        icon.textContent = '🌙';
        label.textContent = 'Dark';
        metaTheme.content = '#1a1a2e';
    }
}

function toggleTheme() {
    const current = document.documentElement.getAttribute('data-theme') || 'dark';
    setTheme(current === 'dark' ? 'light' : 'dark');
}

// Tab switching
function switchTab(tabName) {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));

    document.querySelector(`.tab[onclick="switchTab('${tabName}')"]`).classList.add('active');
    document.getElementById(`tab-${tabName}`).classList.add('active');
}

// Sub-tab switching
function switchSubTab(subTabName) {
    document.querySelectorAll('.sub-tab').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.sub-tab-content').forEach(c => c.classList.remove('active'));

    document.querySelector(`.sub-tab[onclick="switchSubTab('${subTabName}')"]`).classList.add('active');
    document.getElementById(`subtab-${subTabName}`).classList.add('active');

    if (subTabName === 'schema' && schemaData.length === 0) {
        loadSchema();
    }
}

// Note: showStatus is defined in wifi-shared.js (loaded first)
