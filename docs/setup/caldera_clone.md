
    mkdir activision 
    cd activision

    git clone https://github.com/Activision/caldera.git

    cd caldera
    
    export PYTHONPATH="/Users/sgoda/code/aswf/openusd_install/lib/python:$PYTHONPATH"
    export PATH="/Users/sgoda/code/aswf/openusd_install/bin:$PATH"

    ~/code/aswf/openusd_install/bin/usdview ~/code/activision/caldera/caldera.usda


    sgoda@Mac aswf % ./openusd_install/bin/usdtree --simple --metadata ../activision/caldera/caldera.usda
    /
     `--world
         `--mp_wz_island
             `--mp_wz_island_paths
                 `--mp_wz_island_geo
                     |--map_phosphate_mine
                     |     `--(variantSelection)
                     |--map_tile_p
                     |     `--(variantSelection)
                     |--map_infil_ch3
                     |     `--(variantSelection)
                     |--map_gulags
                     |     `--(variantSelection)
                     |--map_tile_d
                     |     `--(variantSelection)
                     |--map_tile_c
                     |     `--(variantSelection)
                     |--map_exfil_ch3
                     |     `--(variantSelection)
                     |--map_tile_e
                     |     `--(variantSelection)
                     |--map_tile_f
                     |     `--(variantSelection)
                     |--map_tile_h
                     |     `--(variantSelection)
                     |--map_tile_j
                     |     `--(variantSelection)
                     |--map_tile_k
                     |     `--(variantSelection)
                     |--map_tile_l
                     |     `--(variantSelection)
                     |--map_tile_o
                     |     `--(variantSelection)
                     |--map_ruins
                     |     `--(variantSelection)
                     |--mv_intel
                     |     `--(variantSelection)
                     |--map_vista
                     |     `--(variantSelection)
                     |--map_village
                     |     `--(variantSelection)
                     |--map_tile_i
                     |     `--(variantSelection)
                     |--map_tile_b
                     |     `--(variantSelection)
                     |--map_airstrip
                     |     `--(variantSelection)
                     |--map_docks
                     |     `--(variantSelection)
                     |--map_tile_g
                     |     `--(variantSelection)
                     |--st_main
                     |   |--st_b
                     |   |     `--(variantSelection)
                     |   |--st_d
                     |   |     `--(variantSelection)
                     |   |--st_c
                     |   |     `--(variantSelection)
                     |   |--st_f
                     |   |     `--(variantSelection)
                     |   |--st_e
                     |   |     `--(variantSelection)
                     |   |--st_h
                     |   |     `--(variantSelection)
                     |   |--st_g
                     |   |     `--(variantSelection)
                     |   |--st_j
                     |   |     `--(variantSelection)
                     |   |--st_i
                     |   |     `--(variantSelection)
                     |   |--st_l
                     |   |     `--(variantSelection)
                     |   |--st_k
                     |   |     `--(variantSelection)
                     |   |--st_n
                     |   |     `--(variantSelection)
                     |   |--st_p
                     |   |     `--(variantSelection)
                     |   `--st_o
                     |         `--(variantSelection)
                     |--map_beachhead
                     |     `--(variantSelection)
                     |--map_subpen
                     |     `--(variantSelection)
                     |--map_tile_n
                     |     `--(variantSelection)
                     |--map_agricultural_center
                     |     `--(variantSelection)
                     |--map_airfield
                     |     `--(variantSelection)
                     |--map_arsenal
                     |     `--(variantSelection)
                     |--map_capital
                     |     `--(variantSelection)
                     `--map_caldera
                           `--(variantSelection)