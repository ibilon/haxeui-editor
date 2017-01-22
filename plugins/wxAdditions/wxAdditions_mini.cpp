///////////////////////////////////////////////////////////////////////////////
//
// HaxeUI-editor - A visual UI editor for HaxeUI.
// Copyright (C) 2016 Valentin Lemière
// 
// Based on code from wxFormBuilder by José Antonio Hurtado
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Written by
//   José Antonio Hurtado - joseantonio.hurtado@gmail.com
//   Juan Antonio Ortega  - jortegalalmolda@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

// wxFormBuilder SDK
#include "component.h"
#include "plugin.h"
#include "xrcconv.h"
#include <ticpp.h>

// wxFlatNotebook
#include <wx/wxFlatNotebook/wxFlatNotebook.h>
#include <wx/wxFlatNotebook/xh_fnb.h>

// wxPropertyGrid
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

// wxWidgets
#include <wx/xrc/xmlres.h>

///////////////////////////////////////////////////////////////////////////////

/**
Event handler for events generated by controls in this plugin
*/
class ComponentEvtHandler : public wxEvtHandler
{
private:
	wxWindow* m_window;
	IManager* m_manager;

public:
	ComponentEvtHandler( wxWindow* win, IManager* manager )
	:
	m_window( win ),
	m_manager( manager ) {}

protected:
	// Enable folding for wxScintilla
	//void OnMarginClick ( wxScintillaEvent& event );

	void OnFlatNotebookPageChanged( wxFlatNotebookEvent& event )
	{
		// Only handle events from this book - prevents problems with nested books, because OnSelected is fired on an
		// object and all of its parents
		if ( m_window != event.GetEventObject() )
		{
			return;
		}

		int selPage = event.GetSelection();
		if ( selPage < 0 )
		{
			return;
		}

		size_t count = m_manager->GetChildCount( m_window );
		for ( size_t i = 0; i < count; i++ )
		{
			wxObject* wxChild = m_manager->GetChild( m_window, i );
			IObject*  iChild = m_manager->GetIObject( wxChild );
			if ( iChild )
			{
				if ( (int)i == selPage && !iChild->GetPropertyAsInteger( _("select") ) )
				{
					m_manager->ModifyProperty( wxChild, _("select"), wxT("1"), false );
				}
				else if ( (int)i != selPage && iChild->GetPropertyAsInteger( _("select") ) )
				{
					m_manager->ModifyProperty( wxChild, _("select"), wxT("0"), false );
				}
			}
		}

		// Select the corresponding panel in the object tree
		wxFlatNotebook* book = wxDynamicCast( m_window, wxFlatNotebook );
		if ( NULL != book )
		{
			m_manager->SelectObject( book->GetPage( selPage ) );
		}
	}

	void OnFlatNotebookPageClosing( wxFlatNotebookEvent& event )
	{
		wxMessageBox( wxT("wxFlatNotebook pages can normally be closed.\nHowever, it is difficult to design a page that has been closed, so this action has been vetoed."),
						wxT("Page Close Vetoed!"), wxICON_INFORMATION, NULL );
		event.Veto();
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( ComponentEvtHandler, wxEvtHandler )
	EVT_FLATNOTEBOOK_PAGE_CHANGED( -1, ComponentEvtHandler::OnFlatNotebookPageChanged )
	EVT_FLATNOTEBOOK_PAGE_CLOSING( -1, ComponentEvtHandler::OnFlatNotebookPageClosing )
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////

class PropertyGridComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		wxPropertyGrid* pg = new wxPropertyGrid((wxWindow *)parent,-1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")),
			obj->GetPropertyAsInteger(_("style")) | obj->GetPropertyAsInteger(_("window_style")));

		if ( !obj->GetPropertyAsString(_("extra_style")).empty() )
		{
			pg->SetExtraStyle( obj->GetPropertyAsInteger( _("extra_style") ) );
		}

		pg->AppendCategory(wxT("Sample Category"));

		// Add string property
		pg->Append( wxStringProperty(wxT("Label"),wxT("Name"),wxT("Initial Value")) );

		// Add int property
		pg->Append ( wxIntProperty ( wxT("IntProperty"), wxPG_LABEL, 12345678 ) );

		// Add float property (value type is actually double)
		pg->Append ( wxFloatProperty ( wxT("FloatProperty"), wxPG_LABEL, 12345.678 ) );

		// Add a bool property
		pg->Append ( wxBoolProperty ( wxT("BoolProperty"), wxPG_LABEL, false ) );
		pg->Append ( wxBoolProperty ( wxT("BoolPropertyAsCheckbox"), wxPG_LABEL, true ) );
		pg->SetPropertyAttribute( wxT("BoolPropertyAsCheckbox"), wxPG_BOOL_USE_CHECKBOX, (long)1);

		// A string property that can be edited in a separate editor dialog.
		pg->Append ( wxLongStringProperty (wxT("LongStringProperty"),
			wxPG_LABEL,
			wxT("This is much longer string than the ")
			wxT("first one. Edit it by clicking the button.")));

		// String editor with dir selector button.
		pg->Append ( wxDirProperty( wxT("DirProperty"), wxPG_LABEL, ::wxGetUserHome()) );

		// A file selector property.
		wxPGId fid = pg->Append ( wxFileProperty( wxT("FileProperty"), wxPG_LABEL, wxEmptyString ) );

		pg->AppendCategory( wxT("Sample Parent Property") );
		wxPGId pid = pg->Append( wxParentProperty(wxT("Car"),wxPG_LABEL) );
		pg->AppendIn( pid, wxStringProperty(wxT("Model"), wxPG_LABEL,wxT("Lamborghini Diablo SV")) );
		pg->AppendIn( pid, wxIntProperty(wxT("Engine Size (cc)"), wxPG_LABEL, 5707) );

		wxPGId speedId = pg->AppendIn( pid, wxParentProperty(wxT("Speeds"),wxPG_LABEL) );
		pg->AppendIn( speedId, wxIntProperty(wxT("Max. Speed (mph)"),wxPG_LABEL,300) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("0-100 mph (sec)"),wxPG_LABEL,3.9) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("1/4 mile (sec)"),wxPG_LABEL,8.6) );
		pg->AppendIn( pid, wxIntProperty(wxT("Price ($)"), wxPG_LABEL, 300000) );

		if ( obj->GetPropertyAsInteger( wxT("include_advanced") ) )
		{
			pg->AppendCategory( wxT("Advanced Properties") );
			// wxArrayStringProperty embeds a wxArrayString.
			pg->Append ( wxArrayStringProperty( wxT("Example of ArrayStringProperty"), wxT("ArrayStringProp") ) );

			// Image file property. Wildcard is auto-generated from available
			// image handlers, so it is not set this time.
			pg->Append ( wxImageFileProperty(wxT("Example of ImageFileProperty"), wxT("ImageFileProp")));

			// Font property has sub-properties.
			pg->Append ( wxFontProperty(wxT("Font"), wxPG_LABEL, wxFontPropertyValue()) );

			// Colour property with arbitrary colour.
			pg->Append ( wxColourProperty(wxT("My Colour 1"), wxPG_LABEL, wxColour(242,109,0) ) );

			// System colour property.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 1"), wxPG_LABEL, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)) );

			// System colour property with custom colour.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 2"), wxPG_LABEL, wxColour(0,200,160) ) );

			// Cursor property
			pg->Append ( wxCursorProperty (wxT("My Cursor"), wxPG_LABEL, wxCURSOR_ARROW));
		}

		return pg;
	}

	void Cleanup( wxObject* )
	{
		// Prevent assert for missing event handler
	}
};

class PropertyGridManagerComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		wxPropertyGridManager* pg = new wxPropertyGridManager((wxWindow *)parent, -1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")),
			obj->GetPropertyAsInteger(_("style")) | obj->GetPropertyAsInteger(_("window_style")));

		if ( !obj->GetPropertyAsString(_("extra_style")).empty() )
		{
			pg->SetExtraStyle( obj->GetPropertyAsInteger( _("extra_style") ) );
		}

		// Adding a page sets target page to the one added, so
		// we don't have to call SetTargetPage if we are filling
		// it right after adding.
		pg->AddPage(wxT("First Page"));

		pg->AppendCategory( wxT("Sample Category") );

		// Add string property
		wxPGId id = pg->Append( wxStringProperty(wxT("Label"),wxT("Name"),wxT("Initial Value")) );
		pg->SetPropertyHelpString( id, wxT("A string property") );

		// Add int property
		pg->Append ( wxIntProperty ( wxT("IntProperty"), wxPG_LABEL, 12345678 ) );

		// Add float property (value type is actually double)
		pg->Append ( wxFloatProperty ( wxT("FloatProperty"), wxPG_LABEL, 12345.678 ) );

		// Add a bool property
		pg->Append ( wxBoolProperty ( wxT("BoolProperty"), wxPG_LABEL, false ) );
		pg->Append ( wxBoolProperty ( wxT("BoolPropertyAsCheckbox"), wxPG_LABEL, true ) );
		pg->SetPropertyAttribute( wxT("BoolPropertyAsCheckbox"), wxPG_BOOL_USE_CHECKBOX, (long)1);

		// Add an enum property
        wxArrayString strings;
        strings.Add(wxT("Herbivore"));
        strings.Add(wxT("Carnivore"));
        strings.Add(wxT("Omnivore"));

        pg->Append( wxEnumProperty(wxT("EnumProperty"), wxPG_LABEL, strings) );

		pg->AppendCategory( wxT("Low Priority Properties") );

		// A string property that can be edited in a separate editor dialog.
		pg->TogglePropertyPriority( pg->Append ( wxLongStringProperty (wxT("LongStringProperty"),
			wxPG_LABEL,
			wxT("This is much longer string than the ")
			wxT("first one. Edit it by clicking the button."))));

		// String editor with dir selector button.
		pg->TogglePropertyPriority( pg->Append ( wxDirProperty( wxT("DirProperty"), wxPG_LABEL, ::wxGetUserHome()) ));

		// A file selector property.
		pg->TogglePropertyPriority( pg->Append ( wxFileProperty( wxT("FileProperty"), wxPG_LABEL, wxEmptyString ) ));

		pg->AddPage(wxT("Second Page"));

		pg->AppendCategory( wxT("Sample Parent Property") );
		wxPGId pid = pg->Append( wxParentProperty(wxT("Car"),wxPG_LABEL) );
		pg->AppendIn( pid, wxStringProperty(wxT("Model"), wxPG_LABEL,wxT("Lamborghini Diablo SV")) );
		pg->AppendIn( pid, wxIntProperty(wxT("Engine Size (cc)"), wxPG_LABEL, 5707) );

		wxPGId speedId = pg->AppendIn( pid, wxParentProperty(wxT("Speeds"),wxPG_LABEL) );
		pg->AppendIn( speedId, wxIntProperty(wxT("Max. Speed (mph)"),wxPG_LABEL,300) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("0-100 mph (sec)"),wxPG_LABEL,3.9) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("1/4 mile (sec)"),wxPG_LABEL,8.6) );
		pg->AppendIn( pid, wxIntProperty(wxT("Price ($)"), wxPG_LABEL, 300000) );

		if ( obj->GetPropertyAsInteger( wxT("include_advanced") ) )
		{
			pg->AppendCategory( wxT("Advanced Properties") );
			// wxArrayStringProperty embeds a wxArrayString.
			pg->Append ( wxArrayStringProperty(wxT("Example of ArrayStringProperty"),
				wxT("ArrayStringProp")));

			// Image file property. Wildcard is auto-generated from available
			// image handlers, so it is not set this time.
			pg->Append ( wxImageFileProperty(wxT("Example of ImageFileProperty"), wxT("ImageFileProp")));

			// Font property has sub-properties.
			pg->Append ( wxFontProperty(wxT("Font"), wxPG_LABEL, wxFontPropertyValue()) );

			// Colour property with arbitrary colour.
			pg->Append ( wxColourProperty(wxT("My Colour 1"), wxPG_LABEL, wxColour(242,109,0) ) );

			// System colour property.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 1"), wxPG_LABEL, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)) );

			// System colour property with custom colour.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 2"), wxPG_LABEL, wxColour(0,200,160) ) );

			// Cursor property
			pg->Append ( wxCursorProperty (wxT("My Cursor"), wxPG_LABEL, wxCURSOR_ARROW));
		}

		// For total safety, finally reset the target page.
		pg->SetTargetPage(0);

		return pg;
	}

	void Cleanup( wxObject* )
	{
		// Prevent assert for missing event handler
	}
};

///////////////////////////////////////////////////////////////////////////////

class FlatNotebookComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		wxFlatNotebook* book = new wxFlatNotebook((wxWindow *)parent,-1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")),
			obj->GetPropertyAsInteger(_("style")) | obj->GetPropertyAsInteger(_("window_style")));

		if ( obj->GetPropertyAsInteger( _("has_images") ) != 0 )
		{
			wxFlatNotebookImageList* images = new wxFlatNotebookImageList();
			book->SetImageList( images );
		}

		book->SetCustomizeOptions( obj->GetPropertyAsInteger( _("customize_options") ) );

		book->PushEventHandler( new ComponentEvtHandler( book, GetManager() ) );

		return book;
	}

	ticpp::Element* ExportToXrc(IObject *obj)
	{
		ObjectToXrcFilter xrc(obj, _("wxFlatNotebook"), obj->GetPropertyAsString(_("name")));
		xrc.AddWindowProperties();
		return xrc.GetXrcObject();
	}

	ticpp::Element* ImportFromXrc( ticpp::Element* xrcObj )
	{
		XrcToXfbFilter filter(xrcObj, _("wxFlatNotebook"));
		filter.AddWindowProperties();
		return filter.GetXfbObject();
	}
};

class FlatNotebookPageComponent : public ComponentBase
{
public:
	void OnCreated( wxObject* wxobject, wxWindow* wxparent )
	{
		// Easy read-only property access
		IObject* obj = GetManager()->GetIObject( wxobject );
		wxFlatNotebook* book = wxDynamicCast( wxparent, wxFlatNotebook );
		wxWindow* page = wxDynamicCast( GetManager()->GetChild( wxobject, 0 ), wxWindow );

		// Error checking
		if ( !( obj && book && page ) )
		{
			wxLogError( _("FlatNotebookPageComponent is missing its haxeui-editor object(%i), its parent(%i), or its child(%i)"), obj, book, page );
			return;
		}

		// Prevent events during construction - two event handlers have been pushed onto the stack
		// VObjEvtHandler and Component Event handler
		wxEvtHandler* vobjEvtHandler = book->PopEventHandler();
		wxEvtHandler* bookEvtHandler = book->PopEventHandler();

		int selection = book->GetSelection();

		// Apply image to page
		IObject* parentObj = GetManager()->GetIObject( wxparent );
		if ( parentObj->GetPropertyAsInteger( _("has_images") ) != 0 )
		{
			if ( !obj->GetPropertyAsString( _("bitmap") ).empty() )
			{
				wxFlatNotebookImageList* imageList = book->GetImageList();
				if ( parentObj->GetPropertyAsInteger( _("auto_scale_images") ) != 0 )
				{
					wxImage image = obj->GetPropertyAsBitmap( _("bitmap") ).ConvertToImage();
					imageList->Add( image.Scale( 16, 16 ) );
				}
				else
				{
					imageList->Add( obj->GetPropertyAsBitmap( _("bitmap") ) );
				}
				book->AddPage( page, obj->GetPropertyAsString( _("label") ), false, imageList->GetCount() - 1 );
			}
			else
			{
				book->AddPage(page,obj->GetPropertyAsString(_("label")));
			}
		}
		else
		{
			book->AddPage(page,obj->GetPropertyAsString(_("label")));
		}

		if ( obj->GetPropertyAsString( _("select") ) == wxT("0") && selection >= 0 )
		{
			book->SetSelection( selection) ;
		}
		else
		{
			book->SetSelection( book->GetPageCount() - 1 );
		}

		// Restore event handling
		book->PushEventHandler( bookEvtHandler );
		book->PushEventHandler( vobjEvtHandler );
	}

	void OnSelected( wxObject* wxobject )
	{
		// Get actual page - first child
		wxObject* page = GetManager()->GetChild( wxobject, 0 );
		if ( NULL == page )
		{
			return;
		}

		wxFlatNotebook* book = wxDynamicCast( GetManager()->GetParent( wxobject ), wxFlatNotebook );
		if ( book )
		{
			for ( int i = 0; i < book->GetPageCount(); ++i )
			{
				if ( book->GetPage( i ) == page )
				{
					// Prevent infinite event loop
					wxEvtHandler* bookEvtHandler = book->PopEventHandler();
					wxEvtHandler* vobjEvtHandler = book->PopEventHandler();

					// Select Page
					book->SetSelection( i );

					// Restore event handling
					book->PushEventHandler( vobjEvtHandler );
					book->PushEventHandler( bookEvtHandler );
				}
			}
		}
	}

	ticpp::Element* ExportToXrc(IObject *obj)
	{
		ObjectToXrcFilter xrc( obj, _("notebookpage") );
		xrc.AddProperty( _("label"), _("label"), XRC_TYPE_TEXT );
		xrc.AddProperty( _("selected"), _("selected"), XRC_TYPE_BOOL );
		if ( !obj->IsNull( _("bitmap") ) )
		{
			xrc.AddProperty( _("bitmap"), _("bitmap"), XRC_TYPE_BITMAP );
		}
		return xrc.GetXrcObject();
	}

	ticpp::Element* ImportFromXrc( ticpp::Element* xrcObj )
	{
		XrcToXfbFilter filter( xrcObj, _("notebookpage") );
		filter.AddWindowProperties();
		filter.AddProperty( _("selected"), _("selected"), XRC_TYPE_BOOL );
		filter.AddProperty( _("label"), _("label"), XRC_TYPE_TEXT );
		filter.AddProperty( _("bitmap"), _("bitmap"), XRC_TYPE_BITMAP );
		return filter.GetXfbObject();
	}
};

BEGIN_LIBRARY()

// Load additional XRC handlers
// This code is actually in the entry point of the plugin - the function GetComponentLibrary()
// I know this looks funky, but it is perfectly valid
wxXmlResource *res = wxXmlResource::Get();
res->AddHandler( new wxFlatNotebookXmlHandler );

// wxPropertyGrid
WINDOW_COMPONENT("wxPropertyGrid", PropertyGridComponent)
MACRO(wxPG_AUTO_SORT)
MACRO(wxPG_HIDE_CATEGORIES)
MACRO(wxPG_ALPHABETIC_MODE)
MACRO(wxPG_BOLD_MODIFIED)
MACRO(wxPG_SPLITTER_AUTO_CENTER)
MACRO(wxPG_TOOLTIPS)
MACRO(wxPG_HIDE_MARGIN)
MACRO(wxPG_STATIC_SPLITTER)
MACRO(wxPG_STATIC_LAYOUT)
MACRO(wxPG_LIMITED_EDITING)
MACRO(wxPG_EX_INIT_NOCAT)
MACRO(wxPG_DEFAULT_STYLE)
MACRO(wxTAB_TRAVERSAL)

// wxPropertyGridManager
WINDOW_COMPONENT("wxPropertyGridManager", PropertyGridManagerComponent)
MACRO(wxPG_EX_NO_FLAT_TOOLBAR)
MACRO(wxPG_EX_MODE_BUTTONS)
MACRO(wxPG_COMPACTOR)
MACRO(wxPGMAN_DEFAULT_STYLE)
MACRO(wxPG_DESCRIPTION)
MACRO(wxPG_TOOLBAR)

// wxFlatNotebook
WINDOW_COMPONENT("wxFlatNotebook",FlatNotebookComponent)
ABSTRACT_COMPONENT("flatnotebookpage",FlatNotebookPageComponent)
MACRO(wxFNB_VC71)
MACRO(wxFNB_FANCY_TABS)
MACRO(wxFNB_TABS_BORDER_SIMPLE)
MACRO(wxFNB_NO_X_BUTTON)
MACRO(wxFNB_NO_NAV_BUTTONS)
MACRO(wxFNB_MOUSE_MIDDLE_CLOSES_TABS)
MACRO(wxFNB_BOTTOM)
MACRO(wxFNB_NODRAG)
MACRO(wxFNB_VC8)
MACRO(wxFNB_X_ON_TAB)
MACRO(wxFNB_BACKGROUND_GRADIENT)
MACRO(wxFNB_COLORFUL_TABS)
MACRO(wxFNB_DCLICK_CLOSES_TABS)
MACRO(wxFNB_SMART_TABS)
MACRO(wxFNB_DROPDOWN_TABS_LIST)
MACRO(wxFNB_ALLOW_FOREIGN_DND)
MACRO(wxFNB_FF2)
MACRO(wxFNB_CUSTOM_DLG)

// wxFNB Customizatio Options
MACRO(wxFNB_CUSTOM_TAB_LOOK)
MACRO(wxFNB_CUSTOM_ORIENTATION)
MACRO(wxFNB_CUSTOM_FOREIGN_DRAG)
MACRO(wxFNB_CUSTOM_LOCAL_DRAG)
MACRO(wxFNB_CUSTOM_CLOSE_BUTTON)
MACRO(wxFNB_CUSTOM_ALL)

END_LIBRARY()
