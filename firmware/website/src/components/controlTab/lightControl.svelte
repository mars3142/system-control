<script lang="ts">
	import { t } from '../../i18n/store';
	import Card from '../common/card.svelte';
	import Toggle from '../common/toggle.svelte';
	import { controlStore } from '../../stores/controlStore';

	let lightOn = $state(false);
	controlStore.subscribe((state) => {
		if (state) lightOn = state.on;
	});

	function toggleLight(checked: boolean) {
		controlStore.setLight({ on: checked });
	}

	let thunderOn = $state(false);

	function toggleThunder(checked: boolean) {
		thunderOn = checked;
		// TODO: Send command to backend
	}
</script>

<Card title="control.light.title">
	<div class="flex flex-col gap-4">
		<Toggle
			bind:checked={lightOn}
			label={$t("control.light.light")}
			onchange={toggleLight}
		/>
		<div class="hidden">
			<Toggle
				bind:checked={thunderOn}
				label={$t("control.light.thunder")}
				onchange={toggleThunder}
			/>
		</div>
	</div>
</Card>
