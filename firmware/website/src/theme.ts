// src/theme.ts
export function setTheme(theme: 'light' | 'dark') {
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);

    const icon = document.getElementById('theme-icon');
    const label = document.getElementById('theme-label');
    const metaTheme = document.querySelector('meta[name="theme-color"]') as HTMLMetaElement | null;

    if (icon) icon.textContent = theme === 'light' ? '☀️' : '🌙';
    if (label) label.textContent = theme === 'light' ? 'Light' : 'Dark';
    if (metaTheme) metaTheme.content = theme === 'light' ? '#f0f2f5' : '#1a1a2e';
}

export function toggleTheme() {
    const current = document.documentElement.getAttribute('data-theme') as 'light' | 'dark' | null || 'dark';
    setTheme(current === 'dark' ? 'light' : 'dark');
}
