<script lang="ts">
    import { invoke } from "@tauri-apps/api/core";
    import { onMount } from "svelte";
    import { fade, fly } from "svelte/transition";

    const State = {
        START: "START",
        WIFI_SELECT: "WIFI_SELECT",
        CONNECTING: "CONNECTING",
        SUCCESS: "SUCCESS",
        ERROR: "ERROR",
    } as const;

    type StateType = (typeof State)[keyof typeof State];

    let currentState: StateType = State.START;
    let ports: { port_name: String; manufacturer: String | null }[] = [];
    let networks: { ssid: String; bssid: String; rssi: number }[] = [];
    let selectedPort = "";
    let selectedSsid = "";
    let password = "";
    let errorMessage = "";

    async function start() {
        try {
            currentState = State.CONNECTING; // Show loading briefly while scanning

            // Get ports first
            console.log("Listing ports...");
            ports = await invoke("list_ports");
            if (ports.length > 0) {
                selectedPort = ports[0].port_name as string;
            }

            // Get networks
            console.log("Scanning wifi...");
            networks = await invoke("get_wifi_list");
            console.log("Networks found:", networks);

            currentState = State.WIFI_SELECT;
        } catch (e) {
            errorMessage = "Failed to initiate: " + e;
            currentState = State.ERROR;
        }
    }

    async function connect() {
        if (!selectedPort) {
            errorMessage =
                "No serial port selected. Ensure device is connected via USB.";
            currentState = State.ERROR;
            return;
        }
        if (!selectedSsid) {
            alert("Please select a network.");
            return;
        }

        currentState = State.CONNECTING;
        try {
            await invoke("connect_device", {
                port: selectedPort,
                ssid: selectedSsid,
                password: password,
            });
            currentState = State.SUCCESS;
        } catch (e) {
            errorMessage = "Failed to connect: " + e;
            currentState = State.ERROR;
        }
    }

    function retry() {
        currentState = State.START;
        errorMessage = "";
        password = "";
    }

    function closeApp() {
        import("@tauri-apps/api/window").then((mod) => {
            mod.getCurrentWindow().close();
        });
    }

    function loadManager() {
        // TODO: Implement launching the manager application
        // typically via tauri-plugin-shell
        closeApp();
    }
</script>

<main class="container">
    <div class="card" in:fade={{ duration: 300 }}>
        {#if currentState === State.START}
            <div class="step" in:fly={{ y: 20, duration: 400 }}>
                <h1>Learning Buddy Initialization</h1>
                <p class="subtitle">Connect your device to get started.</p>
                <button class="primary-btn" on:click={start}>Start Setup</button
                >
            </div>
        {:else if currentState === State.WIFI_SELECT}
            <div class="step" in:fly={{ y: 20, duration: 400 }}>
                <h2>Network Configuration</h2>

                <div class="input-group">
                    <label for="port-select">Serial Port</label>
                    {#if ports.length > 0}
                        <select id="port-select" bind:value={selectedPort}>
                            {#each ports as p}
                                <option value={p.port_name}
                                    >{p.port_name}
                                    {p.manufacturer
                                        ? `(${p.manufacturer})`
                                        : ""}</option
                                >
                            {/each}
                        </select>
                    {:else}
                        <p class="error-text">
                            No devices found. Please connect via USB.
                        </p>
                        <button class="secondary-btn" on:click={start}
                            >Rescan Ports</button
                        >
                    {/if}
                </div>

                <div class="input-group">
                    <label for="wifi-select">Wi-Fi Network</label>
                    <select id="wifi-select" bind:value={selectedSsid}>
                        {#if networks.length === 0}
                            <option value="" disabled>No networks found</option>
                        {/if}
                        {#each networks as net}
                            <option value={net.ssid}
                                >{net.ssid} ({net.rssi} dBm)</option
                            >
                        {/each}
                    </select>
                </div>

                <div class="input-group">
                    <label for="password">Password</label>
                    <input
                        id="password"
                        type="password"
                        bind:value={password}
                        placeholder="Enter Wi-Fi Password"
                    />
                </div>

                <div class="actions">
                    <button
                        class="primary-btn"
                        on:click={connect}
                        disabled={!selectedPort || !selectedSsid}
                        >Connect</button
                    >
                </div>
            </div>
        {:else if currentState === State.CONNECTING}
            <div class="step center" in:fade>
                <h2>Connecting...</h2>
                <div class="loader"></div>
                <p>Please wait while we configure your Learning Buddy.</p>
            </div>
        {:else if currentState === State.SUCCESS}
            <div class="step center" in:fly={{ y: 20, duration: 400 }}>
                <div class="icon-success">✓</div>
                <h1>Learning Buddy Initialized!</h1>
                <p>Your Learning Buddy has been successfully initialized.</p>
                <div class="actions">
                    <button class="secondary-btn" on:click={closeApp}
                        >Close</button
                    >
                    <button class="primary-btn" on:click={loadManager}
                        >Launch Manager</button
                    >
                </div>
            </div>
        {:else if currentState === State.ERROR}
            <div class="step center" in:fly={{ y: 20, duration: 400 }}>
                <div class="icon-error">!</div>
                <h2 class="error-title">Connection Failed</h2>
                <p class="error-msg">{errorMessage}</p>
                <button class="primary-btn" on:click={retry}>Try Again</button>
            </div>
        {/if}
    </div>
</main>

<style>
    :global(body) {
        margin: 0;
        background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
        font-family: "Outfit", "Inter", system-ui, sans-serif;
        color: #333;
        overflow: hidden;
    }

    .container {
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
        width: 100vw;
    }

    .card {
        background: rgba(255, 255, 255, 0.95);
        backdrop-filter: blur(10px);
        padding: 2.5rem;
        border-radius: 20px;
        box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
        width: 100%;
        max-width: 450px;
        text-align: center;
        min-height: 400px;
        display: flex;
        flex-direction: column;
        justify-content: center;
    }

    h1 {
        font-size: 1.8rem;
        color: #1a1a1a;
        margin-bottom: 0.5rem;
        font-weight: 700;
    }

    h2 {
        font-size: 1.5rem;
        margin-bottom: 1.5rem;
        color: #2c3e50;
    }

    .subtitle {
        color: #666;
        margin-bottom: 2rem;
    }

    .input-group {
        text-align: left;
        margin-bottom: 1.25rem;
    }

    label {
        display: block;
        margin-bottom: 0.5rem;
        font-weight: 600;
        font-size: 0.9rem;
        color: #444;
    }

    select,
    input {
        width: 100%;
        padding: 0.8rem;
        border: 2px solid #e0e0e0;
        border-radius: 10px;
        font-size: 1rem;
        transition: border-color 0.3s box-shadow 0.3s;
        box-sizing: border-box;
    }

    select:focus,
    input:focus {
        border-color: #2a5298;
        outline: none;
        box-shadow: 0 0 0 3px rgba(42, 82, 152, 0.1);
    }

    .primary-btn {
        background: linear-gradient(90deg, #1e3c72 0%, #2a5298 100%);
        color: white;
        border: none;
        padding: 1rem 2rem;
        border-radius: 12px;
        font-size: 1rem;
        font-weight: 600;
        cursor: pointer;
        width: 100%;
        transition:
            transform 0.2s,
            box-shadow 0.2s;
    }

    .primary-btn:hover:not(:disabled) {
        transform: translateY(-2px);
        box-shadow: 0 5px 15px rgba(30, 60, 114, 0.3);
    }

    .primary-btn:disabled {
        opacity: 0.6;
        cursor: not-allowed;
    }

    .secondary-btn {
        background: transparent;
        border: 2px solid #1e3c72;
        color: #1e3c72;
        padding: 0.9rem 2rem;
        border-radius: 12px;
        font-size: 1rem;
        font-weight: 600;
        cursor: pointer;
        width: 100%;
        margin-bottom: 0.8rem;
        transition:
            background 0.2s,
            color 0.2s;
    }

    .secondary-btn:hover {
        background: rgba(30, 60, 114, 0.05);
    }

    .loader {
        border: 4px solid #f3f3f3;
        border-top: 4px solid #2a5298;
        border-radius: 50%;
        width: 40px;
        height: 40px;
        animation: spin 1s linear infinite;
        margin: 2rem auto;
    }

    .icon-success {
        font-size: 4rem;
        color: #27ae60;
        margin-bottom: 1rem;
    }

    .icon-error {
        font-size: 3rem;
        color: #e74c3c;
        margin-bottom: 1rem;
        border: 3px solid #e74c3c;
        border-radius: 50%;
        width: 60px;
        height: 60px;
        line-height: 55px;
        margin: 0 auto 1rem auto;
    }

    .error-text {
        color: #e74c3c;
        font-size: 0.9rem;
    }

    .error-msg {
        color: #e74c3c;
        background: rgba(231, 76, 60, 0.1);
        padding: 1rem;
        border-radius: 8px;
        margin-bottom: 1.5rem;
    }

    @keyframes spin {
        0% {
            transform: rotate(0deg);
        }
        100% {
            transform: rotate(360deg);
        }
    }
</style>
