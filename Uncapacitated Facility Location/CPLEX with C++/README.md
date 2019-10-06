# Optimization: Uncapacitated Facility Location

APSPP contains code to solve the all-pairs shortest path problem to model the distance-based costs.

UFL and UFL_ALT contains code to solve this problem to optimality using CPLEX Concert Technology.

The IP formulation used in UFL contains aggregated constraints and takes considerably longer time to solve. The formulation in UFL_ALT uses disaggregated constraints instead; hence, it is considerably faster.
