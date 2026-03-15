<script lang="ts">
	import { t } from "../../i18n/store";

	export let mode = "simulation";
	export let color = "#000000";
	export let clock: string | null = "12:34 Uhr";
</script>

<div class="bg-card p-6 rounded-lg border border-border shadow-sm">
	<h2 class="text-lg font-semibold mb-4 flex items-center gap-2">
		📊 {$t("control.status.title")}
	</h2>

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
</div>
