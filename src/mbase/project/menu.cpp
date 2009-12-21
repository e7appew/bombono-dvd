//
// mbase/project/menu.cpp
// This file is part of Bombono DVD project.
//
// Copyright (c) 2008-2009 Ilya Murav'jov
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 

#include <mbase/_pc_.h>

#include <mlib/string.h>

#include "menu.h"
#include "table.h"
#include "srl-common.h"

namespace Project
{

Menu IsMenu(MediaItem mi)
{
    return ptr::dynamic_pointer_cast<MenuMD>(mi);
}

MenuMD::MenuMD(): mPrms(AData().GetDefMP())
{}

NameValueT<MenuItemMD> LoadMenuItem(Archieve& ar, MenuMD* mn)
{
    MenuItem mi;

    std::string type = GetValue(ar, "Type");
    if( type == "Frame Item" )
        mi = new FrameItemMD(mn);
    else if( type == "Text Item" )
        mi = new TextItemMD(mn);

    if( !mi )
        throw std::runtime_error("Unknown MenuItem type: " + type);

    return NameValue("MenuItem", *mi);
}

void SerializeReference(Archieve& ar, const char* attr_name, MediaItem& mi)
{
    if( ar.IsLoad() )
    {
        std::string ref;
        ar >> NameValue(attr_name, ref);
        //mi = Ref2Media(ref);
        AData().GetData<MediaRefArr>().push_back(std::make_pair(&mi, ref));
    }
    else
        ar << NameValue(attr_name, Media2Ref(mi));
}

void MenuMD::SerializeImpl(Archieve& ar)
{
    // * параметры
    ar( "Params",  mPrms )
      ( "Color",   color );
    SerializeReference(ar, "BGRef", bgRef);

    // * пункты меню
    {
        ArchieveStackFrame asf(ar, "MenuItems");
        if( ar.IsLoad() )
        {
            using namespace boost;
            ArchieveFunctor<MenuItemMD> fnr =
                MakeArchieveFunctor<MenuItemMD>( lambda::bind(&LoadMenuItem, boost::ref(ar), this) );
            LoadArray(ar, fnr);
        }
        else
        {
            for( Itr itr = itmLst.begin(), end = itmLst.end(); itr != end; ++itr )
                ar << NameValue("MenuItem", **itr);
        }
    }
}

static std::string MakeObjectPath(int idx, const char* type)
{
    ASSERT( idx != NO_HNDL );
    return (str::stream() << type << "." << idx).str();
}

std::string GetMediaRef(MediaItem mi)
{
    return MakeObjectPath(AData().GetML().Find(mi), "Media");
}

std::string GetMenuRef(Menu mi)
{
    return MakeObjectPath(AData().GetMN().Find(mi), "Menu");
}

class RefMaker: public ObjVisitor
{
    public:
        std::string  refStr;

        virtual  void  Visit(StillImageMD& obj) { refStr = GetMediaRef(&obj); }
        virtual  void  Visit(VideoMD& obj)      { refStr = GetMediaRef(&obj); }
        virtual  void  Visit(VideoChapterMD& obj);
        virtual  void  Visit(MenuMD& obj)       { refStr = GetMenuRef(&obj);}

                       // неправильные ссылки
        virtual  void  Visit(FrameItemMD& /*obj*/) { ASSERT(0); }
        virtual  void  Visit(TextItemMD& /*obj*/)  { ASSERT(0); }
};

void RefMaker::Visit(VideoChapterMD& obj)
{
    refStr  = GetMediaRef(obj.owner);
    refStr += (str::stream() << "." << ChapterPosInt(&obj)).str();
}

std::string Media2Ref(MediaItem mi)
{
    std::string ref;
    if( mi )
    {
        RefMaker rm;
        mi->Accept(rm);
        ref = rm.refStr; 
    }
    return ref;
}

int GetRefIndex(const char*& str)
{
    if( !*str || *str != '.' )
        throw std::runtime_error("Bad reference index");
    str++;
    long idx;
    if( !Str::GetLong(idx, str) || (idx < 0) )
        throw std::runtime_error("Bad reference index");

    // след. число, если есть
    const char* next_str = strchr(str, '.');
    if( next_str )
        str = next_str;
    else
        str += strlen(str); // в конец

    return idx;
}

std::string ThrowBadIndex(const char* prefix, int idx)
{
    throw std::runtime_error(
        (str::stream() << prefix << idx).str() );
}

MediaItem TryGetMedia(int idx)
{
    MediaList& ml = AData().GetML();
    if( ml.Size()-1 < idx )
        ThrowBadIndex("Bad reference index(medias overflow): ", idx);
    return ml[idx];
}

Menu TryGetMenu(int idx)
{
    MenuList& ml = AData().GetMN();
    if( ml.Size()-1 < idx )
        ThrowBadIndex("Bad reference index(menus overflow): ", idx);
    return ml[idx];
}

ChapterItem GetChapterByIndex(MediaItem mi, int chp_idx)
{
    VideoItem vi = IsVideo(mi);
    if( !vi )
        throw std::runtime_error("Not a video: " + mi->mdName);

    VideoMD::ListType& lst = vi->List();
    if( (int)lst.size()-1 < chp_idx )
    {
        std::string str = "Bad chapter index for video " + mi->mdName + ": ";
        ThrowBadIndex(str.c_str(), chp_idx);
    }
    return lst[chp_idx];
}

MediaItem Ref2Media(const std::string& ref)
{
    MediaItem mi;
    if( !ref.empty() )
    {
        const char* str = ref.c_str();
        //
        // Формат:
        // Ref      = MediaRef | MenuRef
        // MediaRef = "Media" IntAddr [IntAddr]
        // MenuRef  = "Menu"  IntAddr
        // IntAddr  = "." <число>
        //
        
        if( strncmp("Media", str, 5) == 0 )
        {
            // Медиа
            str += 5;
            mi = TryGetMedia(GetRefIndex(str));
            if( *str ) // глава
            {
                int chp_idx = GetRefIndex(str);
                mi = GetChapterByIndex(mi, chp_idx);
            }
        }
        else if( strncmp("Menu", str, 4) == 0 )
        {
            // Меню
            str += 4;
            mi = TryGetMenu(GetRefIndex(str));
        }
        else
            throw std::runtime_error("Bad reference: " + ref);
    }
    return mi;
}

void MenuItemMD::SerializeImpl(Archieve& ar)
{
    // *
    if( ar.IsSave() )
        ar << NameValue("Type", TypeString());

    // *
    SerializeReference(ar, "Ref", itmRef);

    // *
    ar & NameValue("Placement", itmPlc);
}

void FrameItemMD::SerializeImpl(Archieve& ar)
{
    ar & NameValue("Frame", themeStr);
    MyParent::SerializeImpl(ar);
}

void TextItemMD::SerializeImpl(Archieve& ar)
{
    ar("Font",      fontDsc     )
      ("Underline", isUnderlined)
      ("Color",     color       );
    MyParent::SerializeImpl(ar);
}

Menu MakeMenu(const std::string& name)
{
    Menu menu = new MenuMD;
    menu->mdName = name;
    return menu;
}

Menu MakeMenu(int old_sz)
{
    return MakeMenu(MakeAutoName("Menu", old_sz));
}

} // namespace Project
