<script lang="ts">
  import { page } from "$app/stores";
  import { theme } from "$lib/stores/theme";

  let isMenuOpen = $state(false);
</script>

<nav
  class="fixed top-0 w-full z-50 bg-brand-surface/80 backdrop-blur-md border-b-2 border-brand-text/10 p-4 shadow-lg shadow-brand-accent/5"
>
  <div class="container mx-auto flex justify-between items-center">
    <a href="/" class="text-xl font-bold text-brand-accent">LearningBuddy</a>

    <!-- Mobile Menu Button -->
    <button
      class="md:hidden text-brand-text p-2"
      onclick={() => (isMenuOpen = !isMenuOpen)}
      aria-label="Toggle menu"
    >
      <svg
        xmlns="http://www.w3.org/2000/svg"
        class="w-6 h-6"
        fill="none"
        viewBox="0 0 24 24"
        stroke="currentColor"
      >
        {#if isMenuOpen}
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            stroke-width="2"
            d="M6 18L18 6M6 6l12 12"
          />
        {:else}
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            stroke-width="2"
            d="M4 6h16M4 12h16M4 18h16"
          />
        {/if}
      </svg>
    </button>

    <!-- Desktop Menu + Actions -->
    <div class="hidden md:flex items-center gap-4">
      <div class="flex gap-4">
        <a
          href="/"
          class="hover:text-brand-accent transition-colors {$page.url
            .pathname === '/'
            ? 'text-brand-accent font-bold'
            : 'text-brand-text/70 hover:text-brand-text'}">Home</a
        >
        <a
          href="/about"
          class="hover:text-brand-accent transition-colors {$page.url
            .pathname === '/about'
            ? 'text-brand-accent font-bold'
            : 'text-brand-text/70 hover:text-brand-text'}">About</a
        >
        <a
          href="/hardware"
          class="hover:text-brand-accent transition-colors {$page.url
            .pathname === '/hardware'
            ? 'text-brand-accent font-bold'
            : 'text-brand-text/70 hover:text-brand-text'}">Hardware</a
        >
      </div>

      <button
        onclick={() => ($theme = $theme === "dark" ? "light" : "dark")}
        class="p-2 rounded-full hover:bg-white/10 text-brand-text transition-colors"
        aria-label="Toggle theme"
      >
        {#if $theme === "dark"}
          <!-- Sun Icon -->
          <svg
            xmlns="http://www.w3.org/2000/svg"
            class="w-5 h-5"
            viewBox="0 0 24 24"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            stroke-linecap="round"
            stroke-linejoin="round"
            ><circle cx="12" cy="12" r="5" /><path
              d="M12 1v2M12 21v2M4.2 4.2l1.4 1.4M18.4 18.4l1.4 1.4M1 12h2M21 12h2M4.2 19.8l1.4-1.4M18.4 5.6l1.4-1.4"
            /></svg
          >
        {:else}
          <!-- Moon Icon -->
          <svg
            xmlns="http://www.w3.org/2000/svg"
            class="w-5 h-5"
            viewBox="0 0 24 24"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            stroke-linecap="round"
            stroke-linejoin="round"
            ><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z" /></svg
          >
        {/if}
      </button>

      <a
        href="/signin"
        class="bg-brand-accent text-black px-4 py-2 rounded-md hover:opacity-90 transition-opacity font-bold"
        >Sign In</a
      >
    </div>
  </div>

  <!-- Mobile Menu Dropdown -->
  {#if isMenuOpen}
    <div
      class="md:hidden absolute top-full left-0 w-full bg-brand-surface/95 backdrop-blur-xl border-b-2 border-brand-text/10 p-4 flex flex-col gap-4 shadow-2xl animate-in slide-in-from-top-5"
    >
      <a
        href="/"
        onclick={() => (isMenuOpen = false)}
        class="text-lg p-2 rounded hover:bg-white/5 {$page.url.pathname === '/'
          ? 'text-brand-accent font-bold'
          : 'text-brand-text/70'}">Home</a
      >
      <a
        href="/about"
        onclick={() => (isMenuOpen = false)}
        class="text-lg p-2 rounded hover:bg-white/5 {$page.url.pathname ===
        '/about'
          ? 'text-brand-accent font-bold'
          : 'text-brand-text/70'}">About</a
      >
      <a
        href="/hardware"
        onclick={() => (isMenuOpen = false)}
        class="text-lg p-2 rounded hover:bg-white/5 {$page.url.pathname ===
        '/hardware'
          ? 'text-brand-accent font-bold'
          : 'text-brand-text/70'}">Hardware</a
      >

      <div class="h-px bg-gray-800 my-2"></div>

      <div class="flex items-center justify-between p-2">
        <span class="text-brand-text/70">Theme</span>
        <button
          onclick={() => ($theme = $theme === "dark" ? "light" : "dark")}
          class="p-2 rounded-full hover:bg-white/10 text-brand-text transition-colors border border-gray-700"
          aria-label="Toggle theme"
        >
          {#if $theme === "dark"}
            <svg
              xmlns="http://www.w3.org/2000/svg"
              class="w-5 h-5"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              stroke-linecap="round"
              stroke-linejoin="round"
              ><circle cx="12" cy="12" r="5" /><path
                d="M12 1v2M12 21v2M4.2 4.2l1.4 1.4M18.4 18.4l1.4 1.4M1 12h2M21 12h2M4.2 19.8l1.4-1.4M18.4 5.6l1.4-1.4"
              /></svg
            >
          {:else}
            <svg
              xmlns="http://www.w3.org/2000/svg"
              class="w-5 h-5"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              stroke-linecap="round"
              stroke-linejoin="round"
              ><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z" /></svg
            >
          {/if}
        </button>
      </div>

      <a
        href="/signin"
        onclick={() => (isMenuOpen = false)}
        class="bg-brand-accent text-black px-4 py-3 rounded-md hover:opacity-90 transition-opacity font-bold text-center mt-2"
        >Sign In</a
      >
    </div>
  {/if}
</nav>
