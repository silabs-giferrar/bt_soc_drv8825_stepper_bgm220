#eyJzdGF0ZSI6eyJidWlsZFByZXNldHMiOnt9LCJjdXJyZW50Q29uZkluZGV4IjowfSwiZm9sZGVycyI6WyJzdGVwcGVyX2Rydjg4MjUvc3JjIiwic3RlcHBlcl9kcnY4ODI1L2luYyIsInN0ZXBwZXJfZHJ2ODgyNS9jb25maWciXSwiZmlsZXMiOltdfQ==
include_directories(
	"../stepper_drv8825/src"
	"../stepper_drv8825/inc"
	"../stepper_drv8825/config"
)

target_sources(bt_soc_drv8825_stepper_bgm220 PRIVATE
	"../stepper_drv8825/src/stepper21.c"
	"../stepper_drv8825/src/stepper21_wrap.c"
)
