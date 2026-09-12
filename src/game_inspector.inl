    void draw_inspector() {
        const Rect info{fb_.width()-520,88,502,236};
        fb_.blend_rect(info,{8,7,6},215);fb_.rect(info,gold,1);
        if(!world_.in_bounds(selected_x_,selected_y_)){
            text(fb_,info.x+14,info.y+48,"CLICK A TILE TO INSPECT",pale,2);return;
        }
        const Tile& tile=world_.tile(selected_x_,selected_y_);
        text(fb_,info.x+14,info.y+12,"TILE "+std::to_string(selected_x_)+","+std::to_string(selected_y_),pale,2);
        text(fb_,info.x+14,info.y+34,World::terrain_name(tile.terrain),gold,2);
        text(fb_,info.x+170,info.y+34,World::structure_name(tile.structure),gold,2);
        if(tile.structure==Structure::House){
            const int capacity=world_.house_capacity(selected_x_,selected_y_);
            const int free_workers=std::max(0,int(tile.population)-int(tile.employed));
            text(fb_,info.x+14,info.y+62,World::housing_name(tile.housing_level),gold,3);
            text(fb_,info.x+14,info.y+94,"OCCUPANTS "+std::to_string(tile.population)+" / "+std::to_string(capacity)+"   FREE SPACE "+std::to_string(std::max(0,capacity-int(tile.population))),pale,2);
            text(fb_,info.x+14,info.y+116,"EMPLOYED "+std::to_string(tile.employed)+"   FREE LABOUR "+std::to_string(free_workers),pale,2);
            text(fb_,info.x+14,info.y+138,"FOOD "+std::to_string(tile.food_stock)+"   POTTERY "+std::to_string(tile.pottery_stock),pale,2);
            text(fb_,info.x+14,info.y+160,std::string("ROAD ")+(world_.has_road_access(selected_x_,selected_y_)?"YES":"NO")+"   WATER "+(world_.has_well_service(selected_x_,selected_y_)?"YES":"NO")+"   DES "+std::to_string(world_.house_desirability(selected_x_,selected_y_)),pale,2);
            text(fb_,info.x+14,info.y+190,"NEXT EVOLUTION",gold,2);
            text(fb_,info.x+14,info.y+212,world_.house_evolution_status(selected_x_,selected_y_),pale,1);
        }else if(tile.structure==Structure::Road){
            text(fb_,info.x+14,info.y+78,World::road_level_name(world_.road_level(selected_x_,selected_y_)),gold,3);
            text(fb_,info.x+14,info.y+116,"RECORDED TRAFFIC "+std::to_string(tile.road_traffic),pale,2);
            text(fb_,info.x+14,info.y+146,"REAL WALKERS AND CARTS IMPROVE THIS ROUTE",pale,1);
        }else if(world_.worker_capacity(selected_x_,selected_y_)>0){
            const int assigned=world_.workers_assigned(selected_x_,selected_y_);
            const int active=world_.workers_active(selected_x_,selected_y_);
            const int capacity=world_.worker_capacity(selected_x_,selected_y_);
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
        fb_.clear({47,34,28});
        fb_.fill_rect({0,0,fb_.width(),78},panel);

        const auto sr=speed_rects();
        button(sr[0],paused_?"PLAY":"PAUSE",true,paused_,1);
        button(sr[1],"X1",true,!paused_&&simulation_speed_==1,2);
        button(sr[2],"X2",true,!paused_&&simulation_speed_==2,2);
        button(sr[3],"X4",true,!paused_&&simulation_speed_==4,2);

        const std::string date=std::string(world_.month_name())+" "+std::to_string(world_.year_bc())+" BC";
        text(fb_,fb_.width()/2-70,12,date,gold,3);
        const std::string stats="POP "+std::to_string(world_.population())+
            "  IMM "+std::to_string(world_.immigrants_in_transit())+
            "  EMP "+std::to_string(world_.employed_population())+
            "  FOOD "+std::to_string(world_.total_food())+
            "  POT "+std::to_string(world_.total_pottery())+
            "  TREASURY "+std::to_string(world_.treasury());
        text(fb_,280,48,stats,pale,1);
        if(flat_mode_)text(fb_,fb_.width()-340,50,"FLAT DIAGNOSTIC VIEW",gold,1);
        button(main_menu_rect(),"MAIN MENU",true,main_menu_rect().contains(mx_,my_));

        const int tw=cam_.tile_w(),th=cam_.tile_h();
        const int water_phase=static_cast<int>(atmosphere_time_*5.0);
        for(int sum=0;sum<World::kWidth+World::kHeight-1;++sum){
            for(int y=0;y<World::kHeight;++y){
                const int x=sum-y;if(!world_.in_bounds(x,y))continue;
                const IsoPoint p=cam_.project(x,y);if(p.x<-tw||p.x>fb_.width()+tw||p.y<70-th||p.y>fb_.height()+th)continue;
                const Tile& tile=world_.tile(x,y);const Color ground=terrain_color(tile.terrain,x,y);fb_.diamond(p,tw,th,ground,ground);
                if(tile.terrain==Terrain::Water&&((x+y+water_phase)&3)==0)fb_.line(p.x-tw/5,p.y-1,p.x+tw/5,p.y-1,{73,143,163});
                if(tile.terrain==Terrain::Reeds)for(int k=-2;k<=2;++k)fb_.line(p.x+k*3,p.y,p.x+k*3+1,p.y-10,{38,83,43});
                draw_structure_at(x,y,tile,p,tw,th);
            }
        }
        draw_cloud_shadows();
        draw_agents();

        if(world_.in_bounds(hover_x_,hover_y_)){const IsoPoint p=cam_.project(hover_x_,hover_y_);fb_.diamond_outline(p,tw,th,{255,230,130});}

        draw_minimap();
        draw_inspector();

        const auto tools=tool_buttons();
        const char* labels[11]={"INSPECT","ROAD","HOUSE","FARM","GRANARY","MARKET","WELL","HUNT LODGE","CLAY PIT","POTTER","BULLDOZE"};
        for(int i=0;i<11;++i)button(tools[i],labels[i],true,static_cast<int>(tool_)==i);
        text(fb_,220,fb_.height()-116,"F FLAT VIEW  SPACE PAUSE  JOBS USE REAL RESIDENT LABOUR",pale,1);
    }
