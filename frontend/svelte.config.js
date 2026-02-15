import adapter from '@sveltejs/adapter-static';

/** @type {import('@sveltejs/kit').Config} */
const config = {
	kit: {
		adapter: adapter({
			pages: '../backend/build',
			assets: '../backend/build',
			fallback: 'index.html',
		}),
		prerender: {
			handleUnseenRoutes: 'warn',
		},
	}
};

export default config;
