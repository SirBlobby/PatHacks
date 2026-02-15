<script lang="ts">
    import { page } from "$app/stores";
    import { getCurrentUser } from "$lib/api";
    import Icon from "@iconify/svelte";

    let { isOpen = false } = $props();

    const links = [
        {
            href: "/panel/dashboard",
            label: "Dashboard",
            icon: "mdi:view-dashboard-outline",
        },
        { href: "/panel/devices", label: "Devices", icon: "mdi:devices" },
        {
            href: "/panel/sources",
            label: "Sources",
            icon: "mdi:folder-multiple-outline",
        },
        {
            href: "/panel/plans",
            label: "Plans",
            icon: "mdi:rocket-launch-outline",
        },
        { href: "/panel/settings", label: "Settings", icon: "mdi:cog-outline" },
        {
            href: "/panel/profile",
            label: "Profile",
            icon: "mdi:account-outline",
        },
        { href: "/panel/logout", label: "Logout", icon: "mdi:logout" },
    ];

    let user = $derived(getCurrentUser());
</script>

<aside
    class="w-64 bg-brand-surface/90 backdrop-blur-md border-r-2 border-brand-text/10 h-screen fixed flex flex-col p-4 z-50 overflow-y-auto transition-transform duration-300 shadow-xl shadow-brand-accent/5 md:translate-x-0 {isOpen
        ? 'translate-x-0'
        : '-translate-x-full'}"
>
    <div class="mb-8 px-4 pt-4">
        <a
            href="/"
            class="text-xl font-bold text-brand-accent tracking-tighter hover:opacity-80 transition"
            >LearningBuddy</a
        >
    </div>

    <nav class="flex-1 space-y-1">
        {#each links as link}
            <a
                href={link.href}
                class="flex items-center gap-3 px-4 py-3 rounded-lg transition-all duration-200 {$page.url.pathname.startsWith(
                    link.href,
                )
                    ? 'bg-brand-accent text-black font-semibold shadow-lg shadow-brand-accent/20'
                    : 'text-brand-text/70 hover:bg-brand-text/5 hover:text-brand-text'}"
            >
                <Icon icon={link.icon} class="text-xl" />
                <span class="font-medium">{link.label}</span>
            </a>
        {/each}
    </nav>

    <div class="px-4 py-4 mt-auto border-t border-brand-text/10">
        <div class="flex items-center gap-3">
            <div
                class="w-8 h-8 rounded-full bg-brand-accent flex items-center justify-center text-black text-sm font-bold"
            >
                {user?.name?.charAt(0)?.toUpperCase() || "?"}
            </div>
            <div class="text-sm">
                <div class="text-brand-text font-medium">
                    {user?.name || "User"}
                </div>
                <div class="text-brand-text/50 text-xs">
                    {(user?.plan || "free").charAt(0).toUpperCase() +
                        (user?.plan || "free").slice(1)} Plan
                </div>
            </div>
        </div>
    </div>
</aside>
