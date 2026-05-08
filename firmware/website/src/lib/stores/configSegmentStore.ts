import { writable } from 'svelte/store';
import { requestJson } from '../utils/apiClient';

export interface Segment {
  id: string; // Client-generated ID (index fallback, API does not provide IDs)
  name: string;
  start: number;
  leds: number;
}

// Exact API response type
interface ApiSegmentResponse {
	segments: {
		name: string;
		start: number;
		leds: number;
	}[];
}

function createSegmentStore() {
	const { subscribe, set } = writable<Segment[]>([]);

			  async function fetchSegments() {
				const response = await requestJson<ApiSegmentResponse>('/api/wled/config');
				// Use API response directly
				const segments: Segment[] = response.segments.map((seg, index) => ({
				  id: index.toString(),
				  name: seg.name,
				  start: seg.start,
				  leds: seg.leds
				}));
				set(segments);
			  }

			  async function updateSegments(segments: Segment[]) {
				// Send all segments as array in API format
				const apiPayload = {
				  segments: segments.map(seg => ({
					name: seg.name,
					start: seg.start,
					leds: seg.leds
				  }))
				};
				await requestJson(`/api/wled/config`, {
				  method: 'POST',
				  headers: { 'Content-Type': 'application/json' },
				  body: JSON.stringify(apiPayload)
				});
				await fetchSegments(); // Refresh store after update
			  }

	return {
		subscribe,
		fetchSegments,
		updateSegments
	};
}

export const segmentStore = createSegmentStore();
