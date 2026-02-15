import tailwindcss from '@tailwindcss/vite';
import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';

export default defineConfig({
    plugins: [tailwindcss(), sveltekit()],
    server: {
        host: true,
        proxy: {
            '/api': {
                target: process.env.BACKEND_URL || 'http://localhost:5000',
                changeOrigin: true,
            },
            '/socket.io': {
                target: process.env.BACKEND_URL || 'http://localhost:5000',
                changeOrigin: true,
                ws: true,
            },
        },
    },
});
