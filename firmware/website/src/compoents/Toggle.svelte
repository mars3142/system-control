<script lang="ts">
    const { label, icon, ariaLabel, onClick } = $props<{
        label: string;
        icon: string;
        ariaLabel: string;
        onClick?: () => void;
    }>();
</script>

<button class="toggle" aria-label={ariaLabel} onclick={onClick}>
    <span class="icon" id="icon">{icon}</span>
    <span class="label" id="label">{label}</span>
</button>

<style>
    .toggle {
        display: flex;
        align-items: center;
        gap: 6px;
        background: var(--card-bg);
        padding: 8px 12px;
        border-radius: 20px;
        border: 1px solid var(--border);
        cursor: pointer;
        transition: all 0.2s;
        font-size: 0.85rem;
        color: var(--text-muted);
    }

    .toggle:hover {
        border-color: var(--success);
    }

    .icon {
        font-size: 1.2rem;
    }
</style>
