<script lang="ts">
	import { t } from '../../i18n/store';
	import { threadStore } from '../../stores/threadStore';
	import Card from '../common/card.svelte';
	import Button from '../common/button.svelte';

	function handlePermitJoin() {
		threadStore.startPermitJoin(60); // 60 seconds
	}
</script>

<Card title="thread.title">
	<p class="text-sm text-text-muted mb-4">
		{$t('thread.desc')}
	</p>

	<!-- Network Info Section -->
	{#if $threadStore.networkInfo}
		<div class="mb-4 p-4 bg-background rounded-lg border border-border flex flex-col gap-2">
			<div class="flex justify-between items-center">
				<span class="text-sm font-semibold">Status:</span>
				<span class="text-sm text-success">
					{$t(`thread.status.${$threadStore.networkInfo.status}`)}
				</span>
			</div>
			<div class="flex justify-between items-center">
				<span class="text-sm font-semibold">{$t('thread.network')}</span>
				<span class="text-sm font-mono">{$threadStore.networkInfo.networkName}</span>
			</div>
			<div class="flex justify-between items-center">
				<span class="text-sm font-semibold">{$t('thread.panId')}</span>
				<span class="text-sm font-mono">{$threadStore.networkInfo.panId}</span>
			</div>
			<div class="flex justify-between items-center">
				<span class="text-sm font-semibold">{$t('thread.channel')}</span>
				<span class="text-sm font-mono">{$threadStore.networkInfo.channel}</span>
			</div>
		</div>

		<!-- Commissioning Section -->
		<div class="mt-6 flex flex-col gap-4">
			<Button 
				label={$threadStore.isPermitJoinActive 
					? $t('thread.permitJoinActive').replace('{time}', $threadStore.permitJoinTimeLeft.toString())
					: $t('thread.permitJoin')} 
				ariaLabel="Start Pairing" 
				onClick={handlePermitJoin}
			/>

			{#if $threadStore.isPermitJoinActive || $threadStore.joinedDevicesCount > 0}
				<div class="p-3 bg-success/10 border border-success/30 rounded-lg flex items-center justify-center gap-2">
					{#if $threadStore.isPermitJoinActive}
						<span class="inline-block w-4 h-4 rounded-full border-2 border-success border-t-transparent animate-spin"></span>
					{/if}
					<span class="text-sm font-medium text-success">
						{$t('thread.joinedCount').replace('{count}', $threadStore.joinedDevicesCount.toString())}
					</span>
				</div>
			{/if}
		</div>
	{:else}
		<div class="p-4 bg-background rounded-lg border border-border text-center text-sm text-text-muted italic">
			{$t('thread.noNetwork')}
		</div>
	{/if}
</Card>
