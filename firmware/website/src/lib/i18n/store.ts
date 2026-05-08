import { derived, writable } from 'svelte/store';
import { getInitialLang, type Lang, translations } from './index';

function getLang(): Lang {
	const stored = localStorage.getItem('lang');
	if (stored && stored in translations) return stored as Lang;
	return getInitialLang();
}

export const lang = writable<Lang>(getLang());

function getNestedTranslation(obj: any, path: string): string {
	return path.split('.').reduce((prev, curr) => {
		return prev ? prev[curr] : null;
	}, obj);
}

export const t = derived(lang, $lang => {
	return (key: string) => {
		const translation = getNestedTranslation(translations[$lang], key);
		return translation || '[' + key + ']';
	};
});
