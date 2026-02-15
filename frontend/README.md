# LearningBuddy Frontend

The user interface for LearningBuddy, built with SvelteKit 2 and Svelte 5.

## Features

- **Dashboard** -- Responsive, dark-themed UI with glassmorphism effects and learning stats.
- **AI Text Chat** -- Ask questions about your lecture sources with streaming responses.
- **AI Voice Chat** -- Speak directly with your AI study buddy via ElevenLabs Conversational AI. Includes real-time audio visualization, transcript display, and mic controls.
- **Device Management** -- Pair physical devices, view device details, and browse recordings with live status indicators.
- **Source Management** -- Upload documents (PDF, DOCX, text), view summaries, and launch chat sessions.
- **DeskPet** -- Interactive digital companion that reacts to your learning activity.
- **Marketing Pages** -- Public home, about, and hardware pages.

## Setup

1. **Install Dependencies**:
   ```bash
   bun install
   ```

2. **Development Server**:
   ```bash
   bun run dev
   ```
   Runs on `localhost:5173` with API proxy to `localhost:5000`.

3. **Production Build**:
   Outputs static files to `../backend/build/` for Flask to serve.
   ```bash
   bun run build
   ```

## Key Technologies

- **SvelteKit 2** with `@sveltejs/adapter-static`
- **Svelte 5** runes (`$state`, `$derived`, `$effect`, `$props`)
- **Tailwind CSS v4**
- **TypeScript**
- **@iconify/svelte** for icons
- **@elevenlabs/client** for voice chat SDK
- **Chart.js** for dashboard visualizations
