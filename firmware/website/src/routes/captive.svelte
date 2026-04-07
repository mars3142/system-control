<script lang="ts">
	import { t } from '../i18n/store';
	import Card from '../components/common/card.svelte';

	let ssid = $state('');
	let password = $state('');
	let showPassword = $state(false);
	let statusMessage = $state('');
	let statusType = $state<'success' | 'error' | 'info' | ''>('');
	let countdownInterval: ReturnType<typeof setInterval> | null = null;

	let canConnect = $derived(ssid.trim().length > 0 && password.length > 0);

	function togglePassword() {
		showPassword = !showPassword;
	}

	function showStatus(message: string, type: 'success' | 'error' | 'info') {
		statusMessage = message;
		statusType = type;
		if (type !== 'info') {
			setTimeout(() => {
				statusMessage = '';
				statusType = '';
			}, 5000);
		}
	}

	async function saveWifi() {
		if (!ssid.trim()) {
			showStatus($t('wifi.error.ssid'), 'error');
			return;
		}

		showStatus($t('common.loading'), 'info');

		try {
			const response = await fetch('/api/wifi/config', {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({ ssid: ssid.trim(), password })
			});

			if (response.ok) {
				showStatus($t('wifi.saved'), 'success');
				let countdown = 10;
				if (countdownInterval) clearInterval(countdownInterval);
				countdownInterval = setInterval(() => {
					const text = $t('captive.connecting').replace('{seconds}', String(countdown));
					showStatus(text, 'success');
					countdown--;
					if (countdown < 0) {
						if (countdownInterval) clearInterval(countdownInterval);
						showStatus($t('captive.done'), 'success');
					}
				}, 1000);
			} else {
				const errorData = await response.json().catch(() => ({}));
				showStatus($t('error') + ': ' + (errorData.error || $t('wifi.error.save')), 'error');
			}
		} catch (err: unknown) {
			const message = err instanceof Error ? err.message : String(err);
			showStatus($t('error') + ': ' + message, 'error');
		}
	}
</script>

<Card title="captive.subtitle">
	<div class="mb-4">
		<label for="ssid" class="block mb-2 text-sm font-medium text-text-muted">
			{$t('wifi.ssid.title')}
		</label>
		<input
			type="text"
			id="ssid"
			bind:value={ssid}
			placeholder={$t('wifi.ssid.placeholder')}
			class="w-full px-4 py-3 border-2 border-border rounded-lg bg-input text-text text-base focus:outline-none focus:border-success transition-colors"
		/>
	</div>

	<div class="mb-4">
		<label for="password" class="block mb-2 text-sm font-medium text-text-muted">
			{$t('wifi.password.title')}
		</label>
		<div class="relative flex items-center">
			<input
				type={showPassword ? 'text' : 'password'}
				id="password"
				bind:value={password}
				placeholder={$t('wifi.password.placeholder')}
				class="w-full px-4 py-3 pr-12 border-2 border-border rounded-lg bg-input text-text text-base focus:outline-none focus:border-success transition-colors"
			/>
			<button
				type="button"
				onclick={togglePassword}
				aria-label="Toggle password visibility"
				class="absolute right-3 text-xl text-text-muted cursor-pointer bg-transparent border-none p-1 transition-colors hover:text-text"
			>
				{showPassword ? '🙈' : '👁️'}
			</button>
		</div>
	</div>

	<div class="mt-5">
		<button
			onclick={saveWifi}
			disabled={!canConnect}
			class="w-full py-3 px-5 rounded-lg text-base font-semibold cursor-pointer transition-all flex items-center justify-center gap-2 min-h-12 touch-manipulation bg-success text-white hover:opacity-90 active:scale-[0.98] disabled:opacity-50 disabled:cursor-not-allowed disabled:bg-gray-400"
		>
			{$t('captive.connect')}
		</button>
	</div>

	{#if statusMessage}
		<div
			class="mt-3 text-center rounded-lg px-4 py-3 text-sm border {statusType === 'success'
				? 'bg-success/15 border-success text-success'
				: statusType === 'error'
					? 'bg-error/15 border-error text-error'
					: 'bg-accent border-accent/50 text-text'}"
		>
			{statusMessage}
		</div>
	{/if}

	<div class="bg-accent rounded-lg px-4 py-3 mt-5 text-sm text-text-muted">
		<strong class="text-text">ℹ️ {$t('captive.note.title')}</strong>
		{$t('captive.note.text')}
	</div>
</Card>
