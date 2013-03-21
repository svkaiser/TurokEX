// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------

#include "types.h"
#include "common.h"
#include "rnc.h"
#include "pak.h"
#include "decoders.h"

#pragma warning(disable : 4996)

#define CHUNK_DIRECTORY_SOUNDFX     24
#define CHUNK_SOUNDFX_LOOKUP        0
#define CHUNK_SOUNDFX_DATA          1
#define CHUNK_SOUNDFX_DATA_COUNT    4

typedef struct
{
    int offset;
    int size;
} sndstruct_t;

static byte *sndfxdata = NULL;
static byte **sndnames = NULL;

typedef struct
{
    short waveid;
    short delay;
    short u2;
    byte flags;
    byte u4;
    short random;
    short volume;
    short volStart;
    short volEnd;
    short volRamp;
    short u10;
    short frequency;
    short freqStart;
    short freqEnd;
    short freqRamp;
    short u14;
    short u15;
} soundfx_t;

const char *sndfxnames[] =
{
    "menu_option_select",
    "game_saved",
    "ready_pistol",
    "ready_shotgun",
    "ready_tek_weapon_1",
    "ready_tek_weapon_2",
    "charge_tek_weapon_2",
    "ready_missile_launcher",
    "ready_bfg",
    "knife_swish_1",
    "knife_swish_2",
    "knife_swish_3",
    "bow_stretch",
    "bow_twang",
    "arrow_fly_normal",
    "arrow_fly_tek",
    "pistol_shot",
    "rifle_shot",
    "machine_gun_shot_1",
    "machine_gun_shot_2",
    "grenade_launch",
    "auto_shotgun_shot",
    "riot_shotgun_shot",
    "mini_gun_shot",
    "mini_gun_whir",
    "tek_weapon_1",
    "tek_weapon_2",
    "missile_launch",
    "bfg_shot",
    "blow_gun_shot",
    "hulk_blaster_fire",
    "campaingers_concussion",
    "campaingers_scatter_blast",
    "pendulum_swoosh_1",
    "pendulum_swoosh_2",
    "pendulum_swoosh_3",
    "pendulum_swoosh_4",
    "pendulum_swoosh_5",
    "metal_shell_land",
    "shotgun_shell_land",
    "incoming_projectile_1",
    "incoming_projectile_2",
    "incoming_projectile_3",
    "bullet_ricochet_1",
    "bullet_ricochet_2",
    "bullet_ricochet_3",
    "bullet_ricochet_4",
    "bullet_impact_1",
    "bullet_impact_2",
    "bullet_impact_3",
    "bullet_impact_4",
    "bullet_impact_5",
    "bullet_impact_6",
    "bullet_impact_7",
    "bullet_impact_8",
    "bullet_impact_9",
    "bullet_impact_10",
    "bullet_impact_13",
    "bullet_impact_14",
    "bullet_impact_15",
    "body_armor_impact_1",
    "body_armor_impact_2",
    "body_armor_impact_3",
    "body_armor_impact_4",
    "body_armor_impact_5",
    "explosion_1",
    "explosion_2",
    "explosion_3",
    "fire_rumble",
    "fire_spark_1",
    "fire_spark_2",
    "fire_spark_3",
    "fire_spark_4",
    "fire_spark_5",
    "fire_spark_6",
    "fire_spark_7",
    "fire_spark_8",
    "torch_ignight",
    "electrical_spark_1",
    "electrical_spark_2",
    "electrical_spark_3",
    "electrical_spark_4",
    "electrical_spark_5",
    "electrical_spark_6",
    "pulley_squeak_1",
    "pulley_squeak_2",
    "pulley_squeak_3",
    "pulley_squeak_4",
    "pulley_squeak_5",
    "rope_stretch_1",
    "rope_stretch_2",
    "rope_stretch_3",
    "rope_stretch_4",
    "rope_stretch_5",
    "stonerock_crumble_1",
    "stonerock_crumble_2",
    "stonerock_crumble_3",
    "stonerock_crumble_4",
    "stoneboulder_thud",
    "mud_slosh_1",
    "mud_slosh_2",
    "mud_slosh_3",
    "dirt_debris_1",
    "dirt_debris_2",
    "dirt_debris_3",
    "tree_falling",
    "tree_hit_earth",
    "water_drip_1",
    "water_drip_2",
    "water_drip_3",
    "water_drip_4",
    "water_drip_5",
    "water_splash_1",
    "water_splash_2",
    "water_splash_3",
    "water_splash_4",
    "water_splash_5",
    "underwater_swim_1",
    "underwater_swim_2",
    "underwater_swim_3",
    "underwater_swim_4",
    "underwater_swim_5",
    "bird_call_1",
    "bird_call_2",
    "bird_call_3",
    "bird_call_4",
    "bird_call_5",
    "bird_call_6",
    "bird_call_7",
    "bird_call_8",
    "cricket_chirp",
    "cicaada_chirp",
    "locust_chirp",
    "frog_chirp_1",
    "frog_chirp_2",
    "frog_chirp_3",
    "frog_chirp_4",
    "insect_buzz_1",
    "insect_buzz_2",
    "insect_buzz_3",
    "monkey_chip_scream_1",
    "monkey_chip_scream_2",
    "monkey_chip_scream_3",
    "monkey_chip_scream_4",
    "wild_boar_squeal_1",
    "wild_boar_squeal_2",
    "wild_boar_squeal_3",
    "wild_boar_squeal_4",
    "bird_flushing",
    "brush_rustle_1",
    "brush_rustle_2",
    "brush_rustle_3",
    "brush_rustle_4",
    "wind_blow_1",
    "wind_blow_2",
    "wind_blow_3",
    "wind_blow_4",
    "wind_blow_5",
    "rain_lightly",
    "rain_medium",
    "rain_heavy",
    "thunder_roll_1",
    "thunder_roll_2",
    "thunder_roll_3",
    "thunder_roll_4",
    "lightning_strike_1",
    "lightning_strike_2",
    "lightning_strike_3",
    "river_flow_lightly",
    "river_flow_medium",
    "river_flow_heavy",
    "waterfall_rush",
    "lava_flow",
    "gyser_spouting",
    "stalactites_break",
    "ominous_cave_growl_1",
    "ominous_cave_growl_2",
    "ominous_cave_growl_3",
    "ghostly_catacomb_moan_1",
    "ghostly_catacomb_moan_2",
    "ghostly_catacomb_moan_3",
    "light_jungle_footfall",
    "heavy_jungle_footfall",
    "rope_bridge_footfall",
    "climb_cliff_footfall",
    "swamp_footfall",
    "water_footfall",
    "stone_footfall",
    "metal_footfall",
    "large_thud",
    "fall_into_pit",
    "stone_scrape_on_stone",
    "quicksand_swallow",
    "elevator_mechanism",
    "ceremonial_slab_rise",
    "clay_pot_smash",
    "rocks_squish_in_swamp",
    "doorway_open_close_1",
    "doorway_open_close_2",
    "energy_shield_hum",
    "energy_blast",
    "warp_inout",
    "machine_hum",
    "power_hum",
    "thermal_goggles_on",
    "thermal_goggles_power",
    "health_pickup_1",
    "health_pickup_2",
    "health_pickup_3",
    "ultra_health_pickup",
    "hit_point_pickup",
    "spirituality_pickup",
    "spirituality_death_attack",
    "all_map_pickup",
    "raptor_scream",
    "raptor_alert",
    "raptor_footfall",
    "raptor_injury",
    "raptor_attack",
    "raptor_death",
    "raptor_violent_death",
    "raptor_sniff",
    "tiger_footfall",
    "tiger_roar",
    "tiger_alert",
    "tiger_injury",
    "tiger_attack",
    "tiger_death",
    "tiger_violent_death",
    "tiger_breathe",
    "dimetrodon_footfall",
    "dimetrodon_scream",
    "dimetrodon_alert",
    "dimetrodon_injury",
    "dimetrodon_death",
    "dimetrodon_violent_death",
    "dimetrodon_cough_snarl",
    "moschops_footfall",
    "moschops_scream",
    "moschops_alert",
    "moschops_injury",
    "moschops_attack",
    "moschops_death",
    "moschops_violent_death",
    "moschops_breathe",
    "pteranodn_scream",
    "pteranodn_wing_flap",
    "pteranodn_injury",
    "pteranodn_death",
    "pteranodn_hiss",
    "triceratops_footfall",
    "triceratops_scream",
    "triceratops_alert",
    "triceratops_injury",
    "dragonfly_buzz",
    "dragonfly_clicks_chatter_1",
    "dragonfly_clicks_chatter_2",
    "dragonfly_injury",
    "dragonfly_death",
    "dragonfly_attack",
    "beetle_footfall",
    "beetle_buzz",
    "beetle_clicks_chatter_1",
    "beetle_clicks_chatter_2",
    "beetle_injury",
    "beetle_death",
    "beetle_attack",
    "human_effort_injury_grunt_1",
    "human_effort_injury_grunt_2",
    "human_effort_injury_grunt_3",
    "human_effort_injury_grunt_4",
    "human_effort_injury_grunt_5",
    "human_alert_1",
    "human_alert_2",
    "human_alert_3",
    "human_death_scream_1",
    "human_death_scream_2",
    "human_death_scream_3",
    "human_violent_death_1",
    "human_violent_death_2",
    "human_violent_death_3",
    "ancient_warrior_scream",
    "ancient_warrior_alert",
    "ancient_warrior_attack",
    "ancient_warrior_death",
    "high_priest_teleport",
    "high_priest_spell_attack",
    "subterranean_scream",
    "subterranean_earth_rumble",
    "subterranean_acid_attack",
    "subterranean_acid_impact",
    "subterranean_death",
    "subterranean_violent_death",
    "leaper_footfall",
    "leaper_scream",
    "leaper_alert",
    "leaper_injury",
    "leaper_attack",
    "leaper_death",
    "leaper_violent_death",
    "leaper_chitter",
    "alien_footfall",
    "alien_scream",
    "alien_alert",
    "alien_injury",
    "alien_death",
    "alien_jetpack_malfunction",
    "alien_violent_death",
    "hulk_footfall",
    "hulk_scream",
    "hulk_alert",
    "hulk_injury",
    "hulk_attack",
    "hulk_death",
    "hulk_violent_death",
    "hulk_breathe",
    "robot_footfall",
    "robot_servo_1",
    "robot_servo_2",
    "robot_servo_3",
    "robot_servo_4",
    "robot_voice_fx_1",
    "robot_voice_fx_2",
    "robot_voice_fx_3",
    "robot_short_1",
    "robot_short_2",
    "robot_short_3",
    "robot_shutdown",
    "droid_move",
    "droid_servo_1",
    "droid_servo_2",
    "droid_servo_3",
    "droid_object_located",
    "security_droid_move",
    "security_droid_laser_fire",
    "security_droid_laser_lock_on",
    "security_droid_extinguish",
    "santis_scream_1",
    "mantis_scream_2",
    "mantis_chitter",
    "mantis_injury",
    "mantis_attack",
    "mantis_violent_death",
    "trex_footfall",
    "trex_scream",
    "trex_servo_1",
    "trex_servo_2",
    "trex_servo_3",
    "trex_sniff",
    "trex_eye_laser_fire",
    "trex_fire_breathe",
    "trex_bite_attack",
    "trex_violent_death",
    "longhunter_taunt_1",
    "longhunter_taunt_2",
    "longhunter_taunt_3",
    "campainger_taunt_1",
    "campainger_taunt_2",
    "campainger_rage",
    "campainger_attack",
    "campainger_spell",
    "campainger_shorting_out",
    "campainger_death",
    "throw_tomahawk",
    "tomahawk_glow",
    "tomahawk_impact_flesh",
    "shockwave_weapon_fire",
    "shockwave_weapon_ready",
    "shockwave_weapon_fry",
    "raptor_laser_fire",
    "rotating_lift_servo",
    "pteranodon_fly_swoosh",
    "robot_malfunction",
    "mantis_fireball",
    "mantis_death_rain",
    "footfall_on_wood_plank",
    "toucan_fly_by",
    "antelope_flee",
    "ready_grenade_launcher",
    "minigun_stop",
    "ready_pulse_rifle",
    "ready_assault_rifle",
    "ready_mini_gun",
    "ready_auto_shotgun",
    "reload_missile_launcher",
    "reload_auto_shotgun",
    "generic_1_bullet_pickup",
    "generic_2_shell_pickup",
    "generic_3_energy_pickup",
    "generic_4_non_weapon_pickup",
    "generic_5_grenade_pickup",
    "generic_6_arrow_pickup_",
    "generic_7_rocket_pickup",
    "generic_8_enemy_human_gunfire",
    "generic_9_alien_jetpack_engage",
    "generic_10_turok_injury_1",
    "generic_11_turok_injury_2",
    "generic_12_turok_injury_3",
    "generic_13_turok_injury_4",
    "generic_14_turok_water_injury_1",
    "generic_15_turok_water_injury_2",
    "generic_16_turok_small_water_gasp",
    "generic_17_turok_big_water_gasp",
    "generic_18_turok_normal_death",
    "generic_19_turok_mantis_death",
    "generic_20_turok_t-rex_death",
    "generic_21_turok_jump",
    "generic_22_turok_land",
    "generic_23_turok_climb_1",
    "generic_24_turok_climb_2",
    "generic_25_turok_fall_to_death",
    "generic_26_bird_call_9",
    "generic_27_bird_call_10",
    "generic_28_bird_call_11",
    "generic_29_bird_call_12",
    "generic_30_bird_call_13",
    "generic_31_bird_call_14",
    "generic_32_bird_call_15",
    "generic_33_bird_call_16",
    "generic_34_bird_call_17",
    "generic_35_bird_call_18",
    "generic_36_bird_call_19",
    "generic_37_mettalic_moan_1",
    "generic_38_mettalic_moan_2",
    "generic_39_mettalic_moan_3",
    "generic_40_mettalic_moan_4",
    "generic_41_mettalic_moan_5",
    "generic_42_cave_wing_flutter_w__bat_1",
    "generic_43_cave_wing_flutter_w__bat_2",
    "generic_44_cave_wing_flutter_1",
    "generic_45_cave_wing_flutter_2",
    "generic_46_cave_wing_flutter_3",
    "generic_47_monkey_chirp_5",
    "generic_48_monkey_chirp_6",
    "generic_49_monkey_chirp_7",
    "generic_50_monkey_chirp_8",
    "generic_51_monkey_chirp_9",
    "generic_52_monkey_chirp_10",
    "generic_53_monkey_chirp_11",
    "generic_54_monkey_chirp_12",
    "generic_55_monkey_chirp_13",
    "generic_56_monkey_chirp_14",
    "generic_57_cricket_chirp_2",
    "generic_58_locust_chirp_2",
    "generic_59_cave_wing_flutter_4",
    "generic_60_bat_screech_1",
    "generic_61_bat_screech_2",
    "generic_62_painful_moan_4",
    "generic_63_catacomb_growl_1",
    "generic_64_catacomb_growl_2",
    "generic_65_stone_door_3",
    "generic_66_empty",
    "generic_67_gear_move_1",
    "generic_68_stone_door_4",
    "generic_69_stone_door_5",
    "generic_70_door_thud",
    "generic_71_barrel_door",
    "generic_72_gear_click_1",
    "generic_73_stone_door_6",
    "generic_74_gear_move_2",
    "generic_75_gear_move_noise",
    "generic_76_trigger_plate_click_1",
    "generic_77_iggy_ricochet",
    "generic_78_iggy_hatchet_impact",
    "generic_79_iggy_arrow_fly",
    "generic_80_iggy_arrow_impact",
    "generic_81_kick_impact",
    "generic_82_humvee_idle_'looped'",
    "generic_83_humvee_accelerate_'looped'",
    "generic_84_humvee_stop_accelerate",
    "generic_85_humvee_180_spinout",
    "generic_86_humvee_180_turn",
    "generic_87_humvee_jump",
    "generic_88_kick_swish_1",
    "generic_89_jump_swish_1",
    "generic_90_punch_swoosh",
    "generic_91_sommersault",
    "generic_92_gun_twirl",
    "generic_93_land_from_jump",
    "generic_94_mantis_spit_land",
    "generic_95_mantis_spit_injure",
    "generic_96_tar_bubble_1",
    "generic_97_tar_bubble_2",
    "generic_98_mantis_claw_slash_1",
    "generic_99_mantis_jump_swoosh_1",
    "generic_100_mantis_jump_swoosh_2",
    "generic_101_mantis_wing_flap_1",
    "generic_102_mantis_wing_flap_2",
    "generic_103_mantis_death_2",
    "generic_104_mantis_death_3",
    "generic_105_mantis_death_4",
    "generic_106_mantis_land_on_wall",
    "generic_107_mantis_land_on_floor",
    "generic_108_subterranean_dive_rumble",
    "generic_109_subterranean_dive",
    "generic_110_subterranean_surface",
    "generic_111_subterranean_surface_vox",
    "generic_112_iggy_finger_tick",
    "generic_113_hatchet_fly",
    "generic_114_acclaim_pad",
    "generic_115_turok_bow_stretch",
    "generic_116_turok_wind",
    "generic_117_raptor_thrash_1",
    "generic_118_raptor_thrash_2",
    "generic_119_weak_swoosh_1",
    "generic_120_weak_swoosh_2",
    "generic_121_weak_swoosh_3",
    "generic_122_throw_grenade_swoosh",
    "generic_123_strong_weapon_swoosh_1",
    "generic_124_strong_weapon_swoosh_2",
    "generic_125_spell_swoosh",
    "generic_126_spell_cast_2",
    "generic_127_human_land_from_jump",
    "generic_128_human_fall_grunt",
    "generic_129_human_land_from_fall_2",
    "generic_130_chimp_chat",
    "generic_131_leaper_jump_swoosh",
    "generic_132_leaper_jump_thud",
    "generic_133_underwater_swish_1",
    "generic_134_underwater_swish_2",
    "generic_135_leaper_choke",
    "generic_136_leaper_slosh_1",
    "generic_137_leaper_slosh_2",
    "generic_138",
    "generic_139",
    "generic_140",
    "generic_141",
    "generic_142",
    "generic_143",
    "generic_144",
    "generic_145",
    "generic_146",
    "generic_147",
    "generic_148",
    "generic_149",
    "generic_150",
    "generic_151",
    "generic_152",
    "generic_153",
    "generic_154",
    "generic_155",
    "generic_156",
    "generic_157",
    "generic_158",
    "generic_159",
    "generic_160",
    "generic_161",
    "generic_162",
    "generic_163",
    "generic_164",
    "generic_165",
    "generic_166",
    "generic_167",
    "generic_168",
    "generic_169",
    "generic_170",
    "generic_171",
    "generic_172",
    "generic_173",
    "generic_174",
    "generic_175",
    "generic_176",
    "generic_177",
    "generic_178",
    "generic_179",
    "generic_180",
    "generic_181",
    "generic_182",
    "generic_183",
    "generic_184",
    "generic_185",
    "generic_186",
    "generic_187",
    "generic_188",
    "generic_189",
    "generic_190",
    "generic_191",
    "generic_192",
    "generic_193",
    "generic_194",
    "generic_195",
    "generic_196",
    "generic_197",
    "generic_198",
    "generic_199",
    "generic_200",
    "generic_201",
    "generic_202",
    "generic_203",
    "generic_204",
    "generic_205",
    "generic_206",
    "generic_207",
    "generic_208",
    "generic_209",
    "generic_210",
    "generic_211",
    "generic_212",
    "generic_213",
    "generic_214",
    "generic_215",
    "generic_216",
    "generic_217",
    "generic_218",
    "generic_219",
    "generic_220",
    "generic_221",
    "generic_222",
    "generic_223",
    "generic_224",
    "generic_225",
    "generic_226",
    "generic_227",
    "generic_228",
    "generic_229",
    "generic_230",
    "generic_231",
    "generic_232",
    "generic_233",
    "generic_234",
    "generic_235",
    "generic_236",
    "generic_237",
    "generic_238",
    "generic_239",
    "generic_240",
    "generic_241",
    "generic_242",
    "generic_243",
    "generic_244",
    "generic_245",
    "generic_246",
    "generic_247",
    "generic_248",
    "generic_249",
    "generic_250"
};

static float CoerceFloat(__int16 val)
{
    union
    {
       int i;
       float f;
    } coerce_u;

    coerce_u.i = (val << 16);
    return coerce_u.f;
}

byte *GetSndFxChunk(byte *a1, byte *a2, int a3)
{
    int *ptr = (int*)a1;
    return a2 + ptr[a3 + 1];
}

float ConvertTurokPitchToOAL(short value)
{
    float pitch = -CoerceFloat(value);

    if(pitch <= 1200.0f)
        pitch = 2.0f - (pitch / 1200.0f);
    else
        pitch = (1200.0f / pitch);

    return pitch;
}

void SND_StoreSoundShaders(void)
{
    byte *tmp;
    int *ptr;
    byte *chunk1;
    byte *chunk2;
    int size;
    int i;
    char name[256];

    tmp = Com_GetCartData(cartfile, CHUNK_DIRECTORY_SOUNDFX, &size);
    sndfxdata = RNC_ParseFile(tmp, size, 0);

    chunk1 = GetSndFxChunk(sndfxdata, sndfxdata, CHUNK_SOUNDFX_LOOKUP);
    chunk2 = GetSndFxChunk(sndfxdata, sndfxdata, CHUNK_SOUNDFX_DATA);
    ptr = (int*)chunk1;

    for(i = 0; i < 1000; i++)
    {
        int id = DC_LookupSndFXIndex(ptr + 2, ptr[1], i);

        if(id != -1)
        {
            byte *sndChunk = GetSndFxChunk(chunk2, chunk2, id);
            int count = Com_GetCartOffset(sndChunk, CHUNK_SOUNDFX_DATA_COUNT, 0);
            int j;

            Com_StrcatClear();
            Com_SetDataProgress(count);

            Com_Strcat("sounds[%i] =\n", count);
            Com_Strcat("{\n");
            for(j = 0; j < count; j++)
            {
                soundfx_t *sfx = (soundfx_t*)(sndChunk + 8 + (j * sizeof(soundfx_t)));

                Com_Strcat("    {\n");
                Com_Strcat("        wavefile = \"sounds/waves/%s\"\n", sndnames[sfx->waveid]);
                Com_Strcat("        delay = %i\n", sfx->delay);
                Com_Strcat("        dbFreq = %f\n", ConvertTurokPitchToOAL(sfx->frequency));
                Com_Strcat("        gain = %f\n", (CoerceFloat(sfx->volume) / 32.0f) + 1.0f);
                Com_Strcat("        random = %f\n", (float)sfx->random / 100.0f);
                Com_Strcat("        bInterpGain = %i\n", (sfx->flags & 0x4) != 0);
                Com_Strcat("        bInterpFreq = %i\n", (sfx->flags & 0x2) != 0);
                Com_Strcat("        gainFactorStart = %f\n", CoerceFloat(sfx->volStart));
                Com_Strcat("        gainFactorEnd = %f\n", CoerceFloat(sfx->volEnd));
                Com_Strcat("        gainInterpTime = %i\n", sfx->volRamp);
                Com_Strcat("        freqFactorStart = %f\n", ConvertTurokPitchToOAL(sfx->freqStart));
                Com_Strcat("        freqFactorEnd = %f\n", ConvertTurokPitchToOAL(sfx->freqEnd));
                Com_Strcat("        freqInterpTime = %i\n", sfx->freqRamp);
                Com_Strcat("    }\n");
                Com_UpdateDataProgress();
            }
            Com_Strcat("}\n\n");

            sprintf(name, "sounds/shaders/%s.ksnd", sndfxnames[i]);
            Com_StrcatAddToFile(name);
        }
    }

    Com_Free(&sndfxdata);
}

void SND_StoreSounds(void)
{
    int numsounds;
    sndstruct_t *snds;
    byte *buffer;
    int i;

    Com_GetCartFile(
        "PC Turok Sound File (.11k) \0*.11k\0All Files (*.*)\0*.*\0",
        "Locate TUROKSND.11k");

    if(cartfile == NULL)
        return;

    PK_AddFolder("sounds/");
    PK_AddFolder("sounds/shaders/");
    PK_AddFolder("sounds/waves/");

    numsounds = Com_GetCartOffset(cartfile, 0, 0);

    if(numsounds < 1)
    {
        Com_CloseCartFile();
        return;
    }

    snds = (sndstruct_t*)(cartfile + 8);
    buffer = ((cartfile + 8) + sizeof(sndstruct_t) * numsounds + 4);

    sndnames = (byte**)Com_Alloc(sizeof(byte*) * numsounds);

    for(i = 0; i < numsounds; i++)
    {
        sndnames[i] = strdup(buffer);
        buffer += strlen(sndnames[i]) + 1;
    }

    for(i = 0; i < numsounds; i++)
    {
        char name[256];

        strlwr(sndnames[i]);
        sprintf(name, "sounds/waves/%s", sndnames[i]);
        PK_AddFile(name, buffer + snds[i].offset, snds[i].size, false);
    }

    Com_CloseCartFile();
}
