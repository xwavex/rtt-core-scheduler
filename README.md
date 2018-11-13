# RTT Core Scheduler

The Core Scheduler (CS) extension allows to schedule components, distributed on different cores, while upholding precedence constraints. Each involved core will host one CS. Components that should be executed on a particular core are added to the respective CS. All components are scheduled according to the *flow-preservation*-criterion.

## Usage

```python
# Import the core scheduler (CS) package.
import("rtt-core-scheduler");

# Load the service to coanfigure the CSs.
loadService("this","CoreSchedulerService");

# Instantiate the amount of CSs you need.
loadComponent("CS1", "cosima::CoreScheduler");
setActivityOnCPU("CS1", 0, 5, ORO_SCHED_OTHER, 0); # In this case, the CSs are triggered by their internal activation criterion instead of OROCOS.

loadComponent("CS2", "cosima::CoreScheduler");
setActivityOnCPU("CS2", 0, 5, ORO_SCHED_OTHER, 2);

# Instantiate the components that should be scheduled.
loadComponent("R", "Receiver");
R.configure();

loadComponent("S", "Sender");
S.configure();

loadComponent("SSS", "Sender");
SSS.configure();

# Configure the CSs with the service API.
CoreSchedulerService.setInvolvedCoreScheduler(strings("CS1", "CS2")); # Link the CSs with each other.
CoreSchedulerService.setExecutionOrder("CS1", strings("S", "R")); # CS1 should schedule first S and then R.
CoreSchedulerService.setExecutionOrder("CS2", strings("SSS")); # CS2 should only schedule SSS.
CoreSchedulerService.setLastComponentInPTG("R"); # R is the last component in one schedule iteration (Sense-React Chain).

# Define precedence constraints.
CoreSchedulerService.addPTGFormula("S", "R"); # S needs to be executed before R.
CoreSchedulerService.addPTGFormula("SSS", "R"); # SSS needs to be executed before R.

# Finalize the configurations.
CoreSchedulerService.configure();

# Start the CSs.
CS1.start();
CS2.start();
```

## Dependencies
 - [corlab/rtt-core-extensions](https://github.com/corlab/rtt-core-extensions)
