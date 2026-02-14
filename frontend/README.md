# Frontend 

This is the frontend of our project, it has a home page an about page, hardware page, and a project page. The project is to create a learning buddy device that can record lectures transcript them into text and use AI to summarize them and answer questions about them. The text is also chunked into a vector database for efficient retrieval. The frontend is built in Svelte 5 with TypeScript and Tailwind CSS.

Pages should have sections that are turned into components and used in other pages. TS files for pages and components should be in organized in the lib/ts folder and imported for use in the pages and components. ONLY if the component or page has lots of TS code should it be in the lib/ts folder. Otherwise it should be in the component or page's script tag. 

CSS should also be stored as css files in the lib/css folder and imported for use in the pages and components. ONLY if the component or page has lots of CSS should it be in the lib/css folder. Otherwise it should be in the component or page's style tag. Use the JetBrains nerd font for all text in the static/fonts folder. Use a dark theme with parallax effects. Use canvas for the Shapes and ART built from simple shapes. Do not add any animation let. 

Pages 
- / 
- /about
- /hardware
- /signin 
- /register
- /panel/dashboard
- /panel/devices
- /panel/devices/new
- /panel/devices/[id]
- /panel/recordings
- /panel/recordings/[id]
- /panel/recordings/[id]/chat
- /panel/settings
- /panel/profile
- /panel/logout
