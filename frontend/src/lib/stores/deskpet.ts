import { writable } from "svelte/store";

export type DeskPetExpression = "neutral" | "happy" | "surprised" | "sleepy" | "love" | "confused" | "blush";

interface DeskPetState {
    expression: DeskPetExpression;
    message?: string;
}

function createDeskPetStore() {
    const { subscribe, set, update } = writable<DeskPetState>({
        expression: "neutral",
    });

    let timeout: any;

    return {
        subscribe,
        setExpression: (expr: DeskPetExpression, duration: number = 0) => {
            update(s => ({ ...s, expression: expr }));

            if (timeout) clearTimeout(timeout);

            if (duration > 0) {
                timeout = setTimeout(() => {
                    update(s => ({ ...s, expression: "neutral" }));
                }, duration);
            }
        },
        reset: () => set({ expression: "neutral" })
    };
}

export const deskpet = createDeskPetStore();
