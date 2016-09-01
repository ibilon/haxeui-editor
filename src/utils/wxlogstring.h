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
//   Ryan Mulder - rjmyst3@gmail.com
//
///////////////////////////////////////////////////////////////////////////////
#include <wx/log.h>

class wxLogString : public wxLog
{
private:
    wxString m_str;

public:
    wxLogString(){}

    void DoLogString( const wxChar* str, time_t t )
	{
		m_str += str;
		m_str += wxT("\n");
	}

    void FlushStr( wxString& str )
    {
    	str = m_str;
    	m_str.clear();
	}

    wxString GetStr() const
    {
    	return m_str;
	}
};
