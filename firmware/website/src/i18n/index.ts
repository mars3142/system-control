import de from './de.json';
import en from './en.json';

export const translations = { de, en };

export type Lang = keyof typeof translations;

export function getInitialLang(): Lang {
    const navLang = navigator.language.slice(0, 2);
    if (navLang in translations) return navLang as Lang;
    return 'en';
}
