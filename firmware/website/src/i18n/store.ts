import { writable, derived } from 'svelte/store';
import { translations, getInitialLang, type Lang } from './index';

function getLang(): Lang {
    const stored = localStorage.getItem('lang');
    if (stored && stored in translations) return stored as Lang;
    return getInitialLang();
}

export const lang = writable<Lang>(getLang());

export const t = derived(lang, $lang => {
    return (key: string) => {
        return translations[$lang][key] || key;
    };
});
