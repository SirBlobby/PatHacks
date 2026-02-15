<script lang="ts">
    import { invoke } from "@tauri-apps/api/core";
    import { fade, fly } from "svelte/transition";

    // ============================================================
    // State machine
    // ============================================================

    const Step = {
        START: "START",
        SCANNING: "SCANNING",
        PING: "PING",
        WIFI: "WIFI",
        WIFI_CONNECTING: "WIFI_CONNECTING",
        CONFIG: "CONFIG",
        CONFIG_SENDING: "CONFIG_SENDING",
        SUCCESS: "SUCCESS",
        ERROR: "ERROR",
    } as const;

    type StepType = (typeof Step)[keyof typeof Step];

    let currentStep = $state<StepType>(Step.START);
    let ports = $state<{ port_name: string; manufacturer: string | null }[]>([]);
    let networks = $state<{ ssid: string; bssid: string; rssi: number }[]>([]);
    let selectedPort = $state("");
    let selectedSsid = $state("");
    let wifiPassword = $state("");
    let serverUrl = $state("https://buddy.sirblob.co");
    let deviceKey = $state("");
    let errorMessage = $state("");
    let statusMessage = $state("");

    // ============================================================
    // Step 1: Scan ports and WiFi networks
    // ============================================================

    async function startScan() {
        currentStep = Step.SCANNING;
        statusMessage = "Scanning for devices and networks...";

        try {
            // Scan ports and WiFi in parallel
            const [portList, netList] = await Promise.all([
                invoke<typeof ports>("list_ports"),
                invoke<typeof networks>("get_wifi_list").catch(() => [] as typeof networks),
            ]);

            ports = portList;
            networks = netList;

            if (ports.length > 0) {
                selectedPort = ports[0].port_name;
            }

            if (ports.length === 0) {
                errorMessage = "No serial devices found. Make sure your Learning Buddy is connected via USB.";
                currentStep = Step.ERROR;
                return;
            }

            // Auto-advance to ping
            currentStep = Step.PING;
            await pingDevice();
        } catch (e) {
            errorMessage = `Scan failed: ${e}`;
            currentStep = Step.ERROR;
        }
    }

    // ============================================================
    // Step 2: Ping device to verify communication
    // ============================================================

    async function pingDevice() {
        currentStep = Step.PING;
        statusMessage = "Connecting to device...";

        try {
            await invoke("ping_device", { port: selectedPort });
            // Ping succeeded — move to WiFi step
            currentStep = Step.WIFI;
        } catch (e) {
            errorMessage = `Could not communicate with device on ${selectedPort}: ${e}`;
            currentStep = Step.ERROR;
        }
    }

    // ============================================================
    // Step 3: Send WiFi credentials
    // ============================================================

    async function sendWifi() {
        if (!selectedSsid) return;

        currentStep = Step.WIFI_CONNECTING;
        statusMessage = "Sending WiFi credentials and connecting...";

        try {
            await invoke("send_wifi_config", {
                port: selectedPort,
                ssid: selectedSsid,
                password: wifiPassword,
            });
            // WiFi connected — move to config step
            currentStep = Step.CONFIG;
        } catch (e) {
            errorMessage = `WiFi connection failed: ${e}`;
            currentStep = Step.ERROR;
        }
    }

    // ============================================================
    // Step 4: Send server URL + device key
    // ============================================================

    async function sendConfig() {
        if (!serverUrl.trim() || !deviceKey.trim()) return;

        currentStep = Step.CONFIG_SENDING;
        statusMessage = "Saving server configuration...";

        try {
            await invoke("send_server_config", {
                port: selectedPort,
                serverUrl: serverUrl.trim(),
                deviceKey: deviceKey.trim().toUpperCase(),
            });
            currentStep = Step.SUCCESS;
        } catch (e) {
            errorMessage = `Configuration failed: ${e}`;
            currentStep = Step.ERROR;
        }
    }

    // ============================================================
    // Utility
    // ============================================================

    function retry() {
        currentStep = Step.START;
        errorMessage = "";
        statusMessage = "";
        wifiPassword = "";
    }

    function retryFromWifi() {
        errorMessage = "";
        currentStep = Step.WIFI;
    }

    function retryFromConfig() {
        errorMessage = "";
        currentStep = Step.CONFIG;
    }

    function closeApp() {
        import("@tauri-apps/api/window").then((mod) => {
            mod.getCurrentWindow().close();
        });
    }

    // Derived: are we in a loading state?
    let isLoading = $derived(
        currentStep === Step.SCANNING ||
        currentStep === Step.PING ||
        currentStep === Step.WIFI_CONNECTING ||
        currentStep === Step.CONFIG_SENDING
    );

    // Derived: current step number for progress indicator (1-4)
    let stepNumber = $derived(
        currentStep === Step.START || currentStep === Step.SCANNING || currentStep === Step.PING ? 1 :
        currentStep === Step.WIFI || currentStep === Step.WIFI_CONNECTING ? 2 :
        currentStep === Step.CONFIG || currentStep === Step.CONFIG_SENDING ? 3 :
        currentStep === Step.SUCCESS ? 4 : 0
    );
</script>

<main class="container">
    <div class="card" in:fade={{ duration: 300 }}>
        <!-- Progress bar (shown for steps 1-4, hidden on error) -->
        {#if stepNumber > 0}
            <div class="progress-bar">
                {#each [1, 2, 3, 4] as n}
                    <div class="progress-dot" class:active={n <= stepNumber} class:current={n === stepNumber}></div>
                    {#if n < 4}
                        <div class="progress-line" class:active={n < stepNumber}></div>
                    {/if}
                {/each}
            </div>
            <div class="progress-labels">
                <span class:active={stepNumber >= 1}>Device</span>
                <span class:active={stepNumber >= 2}>WiFi</span>
                <span class:active={stepNumber >= 3}>Server</span>
                <span class:active={stepNumber >= 4}>Done</span>
            </div>
        {/if}

        <!-- ==================== START ==================== -->
        {#if currentStep === Step.START}
            <div class="step" in:fly={{ y: 20, duration: 400 }}>
                <h1>Learning Buddy Setup</h1>
                <p class="subtitle">Connect your ESP32 device via USB to get started.</p>
                <button class="primary-btn" onclick={startScan}>Start Setup</button>
            </div>

        <!-- ==================== LOADING STATES ==================== -->
        {:else if isLoading}
            <div class="step center" in:fade>
                <div class="loader"></div>
                <h2>{statusMessage}</h2>
                <p class="hint">
                    {#if currentStep === Step.WIFI_CONNECTING}
                        This may take up to 15 seconds.
                    {:else if currentStep === Step.SCANNING}
                        Detecting serial ports and WiFi networks...
                    {:else}
                        Please wait...
                    {/if}
                </p>
            </div>

        <!-- ==================== WIFI SELECT ==================== -->
        {:else if currentStep === Step.WIFI}
            <div class="step" in:fly={{ y: 20, duration: 400 }}>
                <h2>WiFi Configuration</h2>
                <p class="subtitle">Select a network for your Learning Buddy.</p>

                <!-- Port selector (in case user needs to change) -->
                <div class="input-group">
                    <label for="port-select">Serial Port</label>
                    <select id="port-select" bind:value={selectedPort}>
                        {#each ports as p}
                            <option value={p.port_name}>
                                {p.port_name}{p.manufacturer ? ` (${p.manufacturer})` : ""}
                            </option>
                        {/each}
                    </select>
                </div>

                <div class="input-group">
                    <label for="wifi-select">WiFi Network</label>
                    <select id="wifi-select" bind:value={selectedSsid}>
                        <option value="" disabled>Select a network</option>
                        {#each networks as net}
                            <option value={net.ssid}>{net.ssid} ({net.rssi} dBm)</option>
                        {/each}
                    </select>
                    {#if networks.length === 0}
                        <p class="field-hint">No networks detected. You can type an SSID manually.</p>
                        <input
                            type="text"
                            bind:value={selectedSsid}
                            placeholder="Enter SSID manually"
                        />
                    {/if}
                </div>

                <div class="input-group">
                    <label for="wifi-password">Password</label>
                    <input
                        id="wifi-password"
                        type="password"
                        bind:value={wifiPassword}
                        placeholder="Enter WiFi password"
                    />
                </div>

                <button
                    class="primary-btn"
                    onclick={sendWifi}
                    disabled={!selectedSsid}
                >
                    Connect to WiFi
                </button>
            </div>

        <!-- ==================== SERVER CONFIG ==================== -->
        {:else if currentStep === Step.CONFIG}
            <div class="step" in:fly={{ y: 20, duration: 400 }}>
                <h2>Server Configuration</h2>
                <p class="subtitle">Enter your Learning Buddy server details and device key.</p>

                <div class="input-group">
                    <label for="server-url">Server URL</label>
                    <input
                        id="server-url"
                        type="url"
                        bind:value={serverUrl}
                        placeholder="https://buddy.sirblob.co"
                    />
                    <p class="field-hint">The URL of your Learning Buddy backend server.</p>
                </div>

                <div class="input-group">
                    <label for="device-key">Device Key</label>
                    <input
                        id="device-key"
                        type="text"
                        bind:value={deviceKey}
                        placeholder="ABC123"
                        maxlength="6"
                        style="text-transform: uppercase; letter-spacing: 0.2em; font-size: 1.3rem; text-align: center;"
                    />
                    <p class="field-hint">The 6-character key from your Learning Buddy web dashboard (Devices page).</p>
                </div>

                <button
                    class="primary-btn"
                    onclick={sendConfig}
                    disabled={!serverUrl.trim() || deviceKey.trim().length < 6}
                >
                    Save Configuration
                </button>
            </div>

        <!-- ==================== SUCCESS ==================== -->
        {:else if currentStep === Step.SUCCESS}
            <div class="step center" in:fly={{ y: 20, duration: 400 }}>
                <div class="icon-success">&#10003;</div>
                <h1>Setup Complete!</h1>
                <p>Your Learning Buddy is configured and ready to record lectures.</p>
                <p class="hint">You can now disconnect the USB cable. The device will connect to WiFi and the server automatically on boot.</p>
                <div class="actions">
                    <button class="primary-btn" onclick={closeApp}>Close</button>
                </div>
            </div>

        <!-- ==================== ERROR ==================== -->
        {:else if currentStep === Step.ERROR}
            <div class="step center" in:fly={{ y: 20, duration: 400 }}>
                <div class="icon-error">!</div>
                <h2 class="error-title">Something went wrong</h2>
                <p class="error-msg">{errorMessage}</p>
                <div class="actions">
                    <button class="primary-btn" onclick={retry}>Start Over</button>
                    {#if stepNumber === 2 || statusMessage.includes("WiFi")}
                        <button class="secondary-btn" onclick={retryFromWifi}>Back to WiFi</button>
                    {/if}
                    {#if stepNumber === 3 || statusMessage.includes("Config")}
                        <button class="secondary-btn" onclick={retryFromConfig}>Back to Config</button>
                    {/if}
                </div>
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
        max-width: 480px;
        text-align: center;
        min-height: 420px;
        display: flex;
        flex-direction: column;
        justify-content: center;
    }

    /* Progress bar */
    .progress-bar {
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 0;
        margin-bottom: 0.3rem;
    }

    .progress-dot {
        width: 12px;
        height: 12px;
        border-radius: 50%;
        background: #ddd;
        transition: background 0.3s, transform 0.3s;
        flex-shrink: 0;
    }

    .progress-dot.active {
        background: #2a5298;
    }

    .progress-dot.current {
        transform: scale(1.3);
        box-shadow: 0 0 0 3px rgba(42, 82, 152, 0.2);
    }

    .progress-line {
        width: 40px;
        height: 3px;
        background: #ddd;
        transition: background 0.3s;
    }

    .progress-line.active {
        background: #2a5298;
    }

    .progress-labels {
        display: flex;
        justify-content: space-between;
        padding: 0 0.5rem;
        margin-bottom: 1.5rem;
        font-size: 0.7rem;
        color: #aaa;
        text-transform: uppercase;
        letter-spacing: 0.05em;
    }

    .progress-labels span {
        width: 50px;
        text-align: center;
    }

    .progress-labels span.active {
        color: #2a5298;
        font-weight: 600;
    }

    h1 {
        font-size: 1.7rem;
        color: #1a1a1a;
        margin-bottom: 0.5rem;
        font-weight: 700;
    }

    h2 {
        font-size: 1.4rem;
        margin-bottom: 0.5rem;
        color: #2c3e50;
    }

    .subtitle {
        color: #666;
        margin-bottom: 1.5rem;
        font-size: 0.95rem;
    }

    .hint {
        color: #888;
        font-size: 0.85rem;
        margin-top: 0.5rem;
    }

    .field-hint {
        color: #999;
        font-size: 0.78rem;
        margin-top: 0.3rem;
        margin-bottom: 0;
    }

    .input-group {
        text-align: left;
        margin-bottom: 1.1rem;
    }

    label {
        display: block;
        margin-bottom: 0.4rem;
        font-weight: 600;
        font-size: 0.88rem;
        color: #444;
    }

    select,
    input {
        width: 100%;
        padding: 0.75rem;
        border: 2px solid #e0e0e0;
        border-radius: 10px;
        font-size: 0.95rem;
        transition: border-color 0.3s, box-shadow 0.3s;
        box-sizing: border-box;
        background: #fff;
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
        padding: 0.9rem 2rem;
        border-radius: 12px;
        font-size: 1rem;
        font-weight: 600;
        cursor: pointer;
        width: 100%;
        margin-top: 0.5rem;
        transition: transform 0.2s, box-shadow 0.2s;
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
        padding: 0.8rem 2rem;
        border-radius: 12px;
        font-size: 0.95rem;
        font-weight: 600;
        cursor: pointer;
        width: 100%;
        margin-top: 0.5rem;
        transition: background 0.2s, color 0.2s;
    }

    .secondary-btn:hover {
        background: rgba(30, 60, 114, 0.05);
    }

    .actions {
        display: flex;
        flex-direction: column;
        gap: 0.5rem;
        margin-top: 1rem;
    }

    .loader {
        border: 4px solid #f3f3f3;
        border-top: 4px solid #2a5298;
        border-radius: 50%;
        width: 40px;
        height: 40px;
        animation: spin 1s linear infinite;
        margin: 1.5rem auto;
    }

    .center {
        display: flex;
        flex-direction: column;
        align-items: center;
    }

    .icon-success {
        font-size: 3.5rem;
        color: #27ae60;
        margin-bottom: 0.5rem;
    }

    .icon-error {
        font-size: 2.5rem;
        color: #e74c3c;
        margin-bottom: 0.5rem;
        border: 3px solid #e74c3c;
        border-radius: 50%;
        width: 55px;
        height: 55px;
        line-height: 52px;
        text-align: center;
    }

    .error-title {
        color: #c0392b;
    }

    .error-msg {
        color: #e74c3c;
        background: rgba(231, 76, 60, 0.08);
        padding: 0.9rem;
        border-radius: 8px;
        margin-bottom: 0.5rem;
        font-size: 0.9rem;
        word-break: break-word;
    }

    @keyframes spin {
        0% { transform: rotate(0deg); }
        100% { transform: rotate(360deg); }
    }
</style>
