# OJ Integration Tests — TODO #8

## Tests

| # | Test | Description |
|---|------|-------------|
| 1 | FullPipelineAccepted | Submit correct code → HTTP 200 + status=accepted |
| 2 | FullPipelineWrongAnswer | Submit wrong code → HTTP 200 + status=wrong_answer |
| 3 | FullPipelineCompileError | Submit syntax error → HTTP 200 + status=compile_error |
| 4 | FullPipelineTimeout | Submit infinite loop → status=time_limit (via HTTP) |
| 5 | FullPipelineMemoryLimit | Submit large alloc → status=runtime_error (via HTTP) |
| 6 | FullPipelineMultipleTestCases | Submit correct code with 3 TCs → all passed |
| 7 | FullPipelinePartialPass | Submit code passing 2/3 TCs → status=wrong_answer |
| 8 | JudgeServerMissingCode | POST /judge without code → 400 |
| 9 | JudgeServerInvalidJson | POST /judge with bad JSON → 400 |
| 10 | JudgeServerInputOutput | Submit code with stdin → stdout match |
| 11 | JudgeServerTestCasesStr | Submit via test_cases_str JSON field |
| 12 | TimeLimitPrecision | Tight timeout (100ms) enforced correctly |
