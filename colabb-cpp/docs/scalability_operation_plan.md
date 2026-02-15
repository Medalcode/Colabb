# Colabb Scalability And Operation Plan

## Objectives

- Improve suggestion quality and consistency.
- Reduce perceived latency for interactive suggestions.
- Make failures observable and recoverable.
- Enable controlled rollout of behavior changes.

## Service Level Objectives (SLO)

- Suggestion latency:
  - p50 <= 800 ms
  - p95 <= 2500 ms
- Suggestion success rate (non-empty suggestion) >= 95%
- UI action consistency:
  - Shortcut conflict rate <= 0.5%
- Error budget:
  - Provider request hard failures <= 2% per day

## Key Metrics

- `suggestion.request.count`
- `suggestion.request.cancelled`
- `suggestion.request.rejected_queue_full`
- `suggestion.latency.ms` (histogram)
- `suggestion.cache.hit_rate`
- `provider.http.status_code` (counter by status)
- `provider.http.timeout.count`
- `provider.http.retry.count`
- `ui.shortcut.consumed` vs `ui.shortcut.propagated`

## Operational Design

1. Request lifecycle:
   - Debounced query creates a request id.
   - Previous queued requests are cancelled.
   - Only latest request id updates UI.
2. Context pipeline:
   - Context detection is cached by cwd with short TTL.
   - Terminal context is isolated per tab.
3. Provider gateway:
   - HTTP timeout and retry with exponential backoff.
   - Retry on 429 and 5xx.
4. Execution safety:
   - Suggestions are inserted, not auto-executed.
   - Future: add risk classification for destructive commands.

## Rollout Plan

1. Stage 1 (dev): enable all changes and collect baseline metrics.
2. Stage 2 (canary): 10% users, compare p95 latency and rejection rate.
3. Stage 3 (ramp): 50% users if no regression > 10%.
4. Stage 4 (full): 100% users and freeze for one release cycle.

## Incident Runbook (Minimum)

- Symptom: high latency
  - Check timeout and retry counters.
  - Verify provider status distribution and queue rejections.
  - Reduce context size and increase cache TTL temporarily.
- Symptom: stale or wrong suggestions
  - Confirm latest-request-id drop behavior in logs.
  - Validate per-tab session log paths.
- Symptom: shortcut conflicts
  - Compare consumed/propagated key metrics.
  - Verify terminal key callback return path.

## Backlog (Next)

- Add structured JSON logging with correlation id per request.
- Add command risk scoring before apply.
- Add provider fallback strategy (primary/secondary).
- Add integration tests around request cancellation and stale response drop.
