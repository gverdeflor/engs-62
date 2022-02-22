# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /thayerfs/home/f00436n/engs62/module5/module5_hw_wrapper/platform.tcl
# 
# OR launch xsct and run below command.
# source /thayerfs/home/f00436n/engs62/module5/module5_hw_wrapper/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {module5_hw_wrapper}\
-hw {/thayerfs/home/f00436n/engs62/module5/module5/module5_hw_wrapper.xsa}\
-out {/thayerfs/home/f00436n/engs62/module5}

platform write
domain create -name {standalone_ps7_cortexa9_0} -display-name {standalone_ps7_cortexa9_0} -os {standalone} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {hello_world}
platform generate -domains 
platform active {module5_hw_wrapper}
domain active {zynq_fsbl}
domain active {standalone_ps7_cortexa9_0}
platform generate -quick
platform generate
platform active {module5_hw_wrapper}
platform config -updatehw {/thayerfs/home/f00436n/engs62/module5/module5/module5_hw_wrapper.xsa}
platform clean
platform generate
platform clean
platform generate
platform clean
platform active {module5_hw_wrapper}
platform config -updatehw {/thayerfs/home/f00436n/engs62/module5/module5/module5_hw_wrapper.xsa}
platform generate
platform generate -domains standalone_ps7_cortexa9_0 
