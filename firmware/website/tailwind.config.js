/** @type {import('tailwindcss').Config} */
export default {
  content: ['./src/**/*.{html,js,svelte,ts}'],
  darkMode: 'media',
  theme: {
    extend: {
      colors: {
        bg: 'var(--bg-color)',
        card: 'var(--card-bg)',
        accent: 'var(--accent)',
        text: 'var(--text)',
        'text-muted': 'var(--text-muted)',
        success: 'var(--success)',
        error: 'var(--error)',
        border: 'var(--border)',
        input: 'var(--input-bg)',
        primary: 'var(--primary)',
      },
      boxShadow: {
        custom: 'var(--shadow)',
      }
    },
  },
  plugins: [],
}
