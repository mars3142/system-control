<script lang="ts">
	import { t } from '../../i18n/store';
	import Card from '../common/card.svelte';
	import { controlStore } from '../../stores/controlStore';

	let mode = $state('simulation');
	let color = $state('#000000');
	let clock: string | null = $state('12:34 Uhr');
	controlStore.subscribe((state) => {
		if (state) {
			mode = state.mode;
			clock = state.clock ?? null;
			if (typeof state.color === 'string') {
				color = state.color;
			} else if (typeof state.color === 'object' && state.color) {
				color = `rgb(${state.color.r},${state.color.g},${state.color.b})`;
			}
		}
	});
</script>

<Card title="control.status.title">
	<div class="flex flex-col sm:flex-row gap-4">
		<div class="flex-1 p-3 bg-background rounded-md border border-border">
			<div class="text-xs text-muted-foreground mb-1">
				{$t("control.status.mode")}
			</div>
			<div class="font-medium flex items-center gap-2">
				{#if mode === "day"}☀️ {$t("mode.day")}
				{:else if mode === "night"}🌙 {$t("mode.night")}
				{:else}🔄 {$t("mode.simulation")}{/if}
			</div>
		</div>

		<div class="flex-1 p-3 bg-background rounded-md border border-border">
			<div class="text-xs text-muted-foreground mb-1">
				{$t("control.status.color")}
			</div>
			<div class="h-6 w-full rounded" style="background: {color};"></div>
		</div>

		{#if mode === "simulation"}
			<div
				class="flex-1 p-3 bg-background rounded-md border border-border"
			>
				<div class="text-xs text-muted-foreground mb-1">
					{$t("control.status.clock")}
				</div>
				<div class="font-medium">
					{#if clock}
						{clock}
					{:else}
						{$t("control.status.stopped")}
					{/if}
				</div>
			</div>
		{/if}
	</div>
</Card>
