--       _________ __                 __
--      /   _____//  |_1____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	unit-bazoo.lua	-	Define the bazoo
--
--	(c) Copyright 2001 - 2005 by Fran�ois Beerten, Lutz Sammer and Crestez Leonard
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

DefineIcon({
	Name = "icon-bazoo",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_bazoo.png"})

DefineAnimations("animations-bazoo", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 1", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 1", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 1", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 1", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 25", "wait 2", 
        "frame 30", "sound bazoo-attack", "attack", "wait 2", 
        "frame 35", "sound bazoo-attack", "wait 2", "frame 0", "wait 150", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 40", "wait 2", "frame 45", "wait 2", 
        "frame 50", "wait 2", "frame 55", "unbreakable end", "wait 2", },
    })
DefineUnitType("unit-bazoo", {
	Name = "Bazoo",
	Image = {"file", GetCurrentLuaPath().."/unit_bazoo.png", "size", {64, 64}},
	Shadow = {"file", GetCurrentLuaPath().."/unit_bazoo_s.png", "size", {64, 64}},
	Animations = "animations-bazoo", Icon = "icon-bazoo",
	Costs = {"time", 40, "titanium", 50, "crystal", 100},
	Speed = 8, HitPoints = 50, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28},
	SightRange = 7, ComputerReactionRange = 6, PersonReactionRange = 6,
	Armor = 2, BasicDamage = 5, PiercingDamage = 15, Missile = "missile-bazoo",
	MaxAttackRange = 6, Priority = 60, Points = 50,
	Corpse = {"unit-dead-body4", 0},
	Type = "land", Demand = 0, 
	RightMouseAction = "attack",
	CanAttack = true, CanTargetLand = true, CanTargetAir = true,
	LandUnit = true, organic = true, SelectableByRectangle = true,
	Sounds = {
		"selected", "bazoo-selected",
		"acknowledge", "bazoo-acknowledge",
		"ready", "bazoo-ready",
		"help", "bazoo-help",
		"dead", "bazoo-die"}
	})
DefineHumanCorpse("bazoo")

DefineAllow("unit-bazoo", "AAAAAAAAAAAAAAAA")
