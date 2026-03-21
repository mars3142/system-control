<script lang="ts">
	export let title: string | undefined;
	import { t } from '../../i18n/store';
</script>

<div class="bg-card p-6 rounded-lg border border-border shadow-sm">
    {#if title}
        <h2 class="text-lg font-semibold mb-4 flex items-center gap-2">
            <span class="inline-block w-1 h-5 bg-success rounded"></span>
            {$t(title)}
        </h2>
    {/if}
    <slot />
</div>
