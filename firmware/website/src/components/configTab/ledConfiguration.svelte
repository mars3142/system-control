<script lang="ts">
	import { onMount } from 'svelte';
	import Card from '../common/card.svelte';
	import Button from '../common/button.svelte';
	import { t } from '../../i18n/store';
	import SegmentRow from './segmentRow.svelte';
	import { segmentStore, type Segment } from '../../stores/configSegmentStore';
	import toast from 'svelte-french-toast';

	let segments = $state<Segment[]>([]);
	let nextId = $state(0);

	onMount(() => {
		const unsub = segmentStore.subscribe((data) => {
			segments = data.map((s) => ({ ...s }));
			nextId = data.length;
		});
		segmentStore.fetchSegments();
		return unsub;
	});

	function addSegment() {
		const id = `new-${nextId++}`;
		segments = [...segments, { id, name: $t('wled.segment.name').replace('{num}', String(nextId)), start: 0, leds: 1 }];
	}

	function findOverlap(): [string, string] | null {
		const sorted = [...segments].sort((a, b) => a.start - b.start);
		for (let i = 0; i < sorted.length - 1; i++) {
			const a = sorted[i];
			const b = sorted[i + 1];
			if (a.start + a.leds > b.start) {
				return [a.name || `#${i + 1}`, b.name || `#${i + 2}`];
			}
		}
		return null;
	}

	async function saveSegments() {
		const overlap = findOverlap();
		if (overlap) {
			toast.error(`"${overlap[0]}" & "${overlap[1]}" überschneiden sich`);
			return;
		}
		try {
			await segmentStore.updateSegments(segments);
			toast.success($t('wled.saved'));
		} catch {
			toast.error($t('wled.error.save'));
		}
	}
</script>

<Card title="wled.config.title">
	<p class="text-sm -mt-3 mb-4 text-text-muted">{$t('wled.config.desc')}</p>

	<div class="flex justify-center items-center mb-2">
		<div class="flex-1 text-sm font-medium">{$t('wled.segments.title')}</div>
		<Button label={$t('wled.segment.add')} ariaLabel={$t('wled.segment.add')} icon="➕" onClick={addSegment} />
	</div>
	<div class="overflow-y-auto max-h-64 scrollbar">
		<table class="w-full table-fixed border-collapse text-left">
			<colgroup>
				<col />
				<col class="w-24" />
				<col class="w-24" />
				<col class="w-12" />
			</colgroup>
			<thead class="sticky top-0 z-10 bg-card">
				<tr class="border-b-2 border-border">
					<th class="px-3 py-2 text-sm">{$t('wled.segment.name').replace('{num}', '')}</th>
					<th class="px-3 py-2 text-sm text-center">{$t('wled.segment.start')}</th>
					<th class="px-3 py-2 text-sm text-center">{$t('wled.segment.leds')}</th>
					<th class="w-12"></th>
				</tr>
			</thead>
			<tbody>
				{#each segments as segment (segment.id)}
					<SegmentRow
						bind:name={segment.name}
						bind:start={segment.start}
						bind:leds={segment.leds}
						onDelete={() => { segments = segments.filter((s) => s.id !== segment.id); }}
					/>
				{/each}
				{#if segments.length === 0}
					<tr>
						<td colspan="3" class="px-3 py-4 text-sm text-center text-text-muted">
							<div>{$t('wled.segments.empty.title')}</div>
							<div class="text-xs mt-1">{$t('wled.segments.empty.hint')}</div>
						</td>
					</tr>
				{/if}
			</tbody>
		</table>
	</div>
	<div class="flex justify-end mt-3">
		<Button label={$t('btn.save')} ariaLabel={$t('btn.save')} icon="💾" onClick={saveSegments} />
	</div>
</Card>

<style>
    .scrollbar {
        scrollbar-color: var(--primary) var(--border);
        scrollbar-width: thin;
    }
    .scrollbar::-webkit-scrollbar {
        width: 6px;
    }
    .scrollbar::-webkit-scrollbar-track {
        background: var(--border);
        border-radius: 3px;
    }
    .scrollbar::-webkit-scrollbar-thumb {
        background: var(--primary);
        border-radius: 3px;
    }
</style>
