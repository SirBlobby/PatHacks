import { writable } from 'svelte/store';
import { browser } from '$app/environment';

const defaultValue = 'dark';
const initialValue = browser ? window.localStorage.getItem('theme') ?? defaultValue : defaultValue;

export const theme = writable(initialValue);

theme.subscribe((value) => {
    if (browser) {
        window.localStorage.setItem('theme', value);
        document.documentElement.setAttribute('data-theme', value);
    }
});
