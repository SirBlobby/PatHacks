// Tauri doesn't have a Node.js server to do proper SSR
// so we will use adapter-static to prerender the app (SSG)
// See: https://v2.tauri.app/start/frontend/sveltekit/ for more info
import adapter from "@sveltejs/adapter-static";
import { vitePreprocess } from "@sveltejs/vite-plugin-svelte";

/** @type {import('@sveltejs/kit').Config} */
const config = {
  preprocess: vitePreprocess(),
  kit: {
    adapter: adapter(),
    files: {
      assets: 'static',
      hooks: {
        client: 'front_end/hooks.client',
        server: 'front_end/hooks.server'
      },
      lib: 'front_end/lib',
      params: 'front_end/params',
      routes: 'front_end/routes',
      serviceWorker: 'front_end/service-worker',
      appTemplate: 'front_end/app.html',
      errorTemplate: 'front_end/error.html'
    }
  },
};

export default config;
