
linefollowerdatadir = $(datadir)/linefollower

# Textures for the virtual camera
textures = carpet.data road_across.data road_dashed1.data road_dashed2.data \
           road.data road_narrow.data road_narrowwide.data road_vcross.data \
           road_widenarrow.data road_zigzag.data wall.data wood.data

# Shaders for GLES2
shaders = fragment.sl vertex.sl

if GLES2
dist_linefollowerdata_DATA = $(textures) $(shaders)
endif

# Configuration file
dist_sysconf_DATA = follow.conf

