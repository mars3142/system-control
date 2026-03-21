<script lang="ts">
	import { t } from '../../i18n/store';
	import ModeButton from './modeButton.svelte';
	import DropDown from '../common/dropDown.svelte';
	import Card from '../common/card.svelte';
	import { controlStore } from '../../stores/controlStore';

	let mode = 'simulation';
	let activeSchema = 'schema_01.csv';
	let schemas = [
		{ value: 'schema_01.csv', label: $t('schema.name.1') },
		{ value: 'schema_02.csv', label: $t('schema.name.2') },
		{ value: 'schema_03.csv', label: $t('schema.name.3') }
	];

	controlStore.subscribe((state) => {
		if (state) {
			mode = state.mode;
			activeSchema = state.schema ?? 'schema_01.csv';
		}
	});

	function setMode(newMode: string) {
		controlStore.setMode({ mode: newMode });
	}

	function handleSchemaChange(event: CustomEvent<string>) {
		controlStore.setSchema({ schema: event.detail });
	}
</script>

<Card title="control.mode.title">
	<div class="flex gap-2 mb-6">
		<ModeButton active={mode === 'day'} icon="☀️" label={$t('mode.day')} onClick={() => setMode('day')} />
		<ModeButton active={mode === 'night'} icon="🌙" label={$t('mode.night')} onClick={() => setMode('night')} />
		<ModeButton active={mode === 'simulation'} icon="🔄" label={$t('mode.simulation')}
								onClick={() => setMode('simulation')} />
	</div>

	{#if mode === 'simulation'}
		<div class="p-4 bg-background rounded-md border border-border">
			<label for="active-schema" class="block text-sm font-medium mb-2">{$t("control.schema.active")}</label>

			<DropDown
				id="active-schema"
				options={schemas}
				bind:value={activeSchema}
				onchange={handleSchemaChange}
			/>
		</div>
	{/if}
</Card>
