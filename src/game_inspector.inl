    void draw_inspector() {
        // An inspector is a contextual graphical overlay, not a permanent
        // desktop-style empty window.  With no selected tile the city remains
        // visually unobstructed.
        if(!world_.in_bounds(selected_x_,selected_y_)) return;

        const Rect info{fb_.width()-520,88,502,236};
        fb_.blend_rect(info,{8,7,6},215);fb_.rect(info,gold,1);

        int inspect_x=selected_x_,inspect_y=selected_y_;
        const Tile& selected=world_.tile(selected_x_,selected_y_);
        if(selected.structure==Structure::House){
            const int ax=world_.residence_anchor_x(selected_x_,selected_y_);
            const int ay=world_.residence_anchor_y(selected_x_,selected_y_);
            if(ax>=0&&ay>=0){inspect_x=ax;inspect_y=ay;}
        }
        const Tile& tile=world_.tile(inspect_x,inspect_y);

        text(fb_,info.x+14,info.y+12,"TILE "+std::to_string(selected_x_)+","+std::to_string(selected_y_),pale,2);
        text(fb_,info.x+14,info.y+34,World::terrain_name(selected.terrain),gold,2);
        text(fb_,info.x+170,info.y+34,World::structure_name(selected.structure),gold,2);

        if(selected.structure==Structure::House){
            const int capacity=world_.house_capacity(inspect_x,inspect_y);
            const int occupants=world_.residence_population(inspect_x,inspect_y);
            const int employed=world_.residence_employed(inspect_x,inspect_y);
            const int free_workers=std::max(0,occupants-employed);
            const int footprint=world_.residence_tiles(inspect_x,inspect_y);
            text(fb_,info.x+14,info.y+58,world_.residence_name(inspect_x,inspect_y),gold,3);
            if(footprint>1){
                text(fb_,info.x+14,info.y+84,
                    "2X2 RESIDENCE  4 TILES  ANCHOR "+std::to_string(inspect_x)+","+std::to_string(inspect_y),pale,1);
            }else{
                text(fb_,info.x+14,info.y+84,"SINGLE-TILE RESIDENCE",pale,1);
            }
            text(fb_,info.x+14,info.y+104,
                "OCCUPANTS "+std::to_string(occupants)+" / "+std::to_string(capacity)+
                "   FREE SPACE "+std::to_string(std::max(0,capacity-occupants)),pale,2);
            text(fb_,info.x+14,info.y+126,
                "EMPLOYED "+std::to_string(employed)+"   FREE LABOUR "+std::to_string(free_workers),pale,2);
            text(fb_,info.x+14,info.y+148,
                "FOOD "+std::to_string(world_.residence_food(inspect_x,inspect_y))+
                "   POTTERY "+std::to_string(world_.residence_pottery(inspect_x,inspect_y)),pale,2);
            text(fb_,info.x+14,info.y+170,
                std::string("ROAD ")+(world_.has_road_access(inspect_x,inspect_y)?"YES":"NO")+
                "   WATER "+(world_.has_well_service(inspect_x,inspect_y)?"YES":"NO")+
                "   DES "+std::to_string(world_.house_desirability(inspect_x,inspect_y)),pale,2);
            text(fb_,info.x+14,info.y+194,"NEXT EVOLUTION",gold,2);
            text(fb_,info.x+14,info.y+214,world_.house_evolution_status(inspect_x,inspect_y),pale,1);
        }else if(tile.structure==Structure::Road){
            text(fb_,info.x+14,info.y+78,World::road_level_name(world_.road_level(inspect_x,inspect_y)),gold,3);
            text(fb_,info.x+14,info.y+116,"RECORDED TRAFFIC "+std::to_string(tile.road_traffic),pale,2);
            text(fb_,info.x+14,info.y+146,"REAL WALKERS AND CARTS IMPROVE THIS ROUTE",pale,1);
        }else if(world_.worker_capacity(inspect_x,inspect_y)>0){
            const int assigned=world_.workers_assigned(inspect_x,inspect_y);
            const int active=world_.workers_active(inspect_x,inspect_y);
            const int capacity=world_.worker_capacity(inspect_x,inspect_y);
            text(fb_,info.x+14,info.y+66,"STAFF "+std::to_string(assigned)+" / "+std::to_string(capacity)+"   AT WORK "+std::to_string(active),gold,2);
            if(assigned==0)text(fb_,info.x+14,info.y+92,"WAITING FOR RESIDENT LABOUR",{226,143,91},2);
            else if(active==0)text(fb_,info.x+14,info.y+92,"STAFF ARE COMMUTING OR AT HOME",pale,1);
            else text(fb_,info.x+14,info.y+92,"PRODUCTION OR DISTRIBUTION ACTIVE",{152,205,126},1);
            if(tile.structure==Structure::ClayPit){
                text(fb_,info.x+14,info.y+122,"CLAY STOCK "+std::to_string(tile.clay_stock),pale,2);
            }else if(tile.structure==Structure::Potter){
                text(fb_,info.x+14,info.y+122,"CLAY "+std::to_string(tile.clay_stock)+"   POTTERY "+std::to_string(tile.pottery_stock),pale,2);
            }else if(tile.structure==Structure::Market){
                text(fb_,info.x+14,info.y+122,"FOOD "+std::to_string(tile.food_stock)+"   POTTERY "+std::to_string(tile.pottery_stock),pale,2);
            }else if(tile.structure==Structure::Farm||tile.structure==Structure::Granary||tile.structure==Structure::HuntingLodge){
                text(fb_,info.x+14,info.y+122,"FOOD STOCK "+std::to_string(tile.food_stock),pale,2);
            }
            text(fb_,info.x+14,info.y+154,"THIS BUILDING USES REAL HOUSEHOLD WORKERS",pale,1);
        }
    }

    void draw_game() {
        // World first. UI is an overlay and must never be overwritten by the
        // isometric map.
        fb_.clear({194,153,88});

        const int tw=cam_.tile_w(),th=cam_.tile_h();
        const int water_phase=static_cast<int>(atmosphere_time_*5.0);

        // Work out which logical coordinates cover the current viewport and
        // draw a presentation-only terrain apron outside the finite simulation
        // grid. This removes the giant diamond/triangle board silhouette while
        // keeping interaction and simulation strictly inside World::kWidth x kHeight.
        std::array<IsoPoint,4> logical_corners{};
        const std::array<IsoPoint,4> screen_corners{{
            {0,0},{fb_.width()-1,0},{0,fb_.height()-1},{fb_.width()-1,fb_.height()-1}
        }};
        for(std::size_t i=0;i<screen_corners.size();++i){
            int tx=0,ty=0;
            cam_.pick(screen_corners[i].x,screen_corners[i].y,tx,ty);
            logical_corners[i]={tx,ty};
        }
        int min_x=logical_corners[0].x,max_x=logical_corners[0].x;
        int min_y=logical_corners[0].y,max_y=logical_corners[0].y;
        for(const auto& c:logical_corners){
            min_x=std::min(min_x,c.x);max_x=std::max(max_x,c.x);
            min_y=std::min(min_y,c.y);max_y=std::max(max_y,c.y);
        }
        min_x-=4;max_x+=4;min_y-=4;max_y+=4;

        for(int sum=min_x+min_y;sum<=max_x+max_y;++sum){
            for(int y=min_y;y<=max_y;++y){
                const int x=sum-y;
                if(x<min_x||x>max_x)continue;
                const IsoPoint p=cam_.project(x,y);
                if(p.x<-tw||p.x>fb_.width()+tw||p.y<-th||p.y>fb_.height()+th)continue;
                const Terrain terrain=presentation_terrain(x,y);
                draw_ground_tile(x,y,terrain,p,tw,th,water_phase);
                if(world_.in_bounds(x,y)){
                    const Tile& tile=world_.tile(x,y);
                    draw_structure_at(x,y,tile,p,tw,th);
                }
            }
        }

        draw_cloud_shadows();
        draw_agents();

        if(world_.in_bounds(hover_x_,hover_y_)){
            const IsoPoint p=cam_.project(hover_x_,hover_y_);
            fb_.diamond_outline(p,tw,th,{255,230,130});
        }

        // Graphical screens are composited over the world last.  The default
        // city view contains no empty desktop-style inspector window, no row of
        // text buttons, and no debug sentence across the bottom.
        draw_top_hud();
        draw_minimap();
        draw_inspector();
        draw_tool_ribbon();
    }
