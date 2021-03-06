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

#ifndef __CODE_WRITER__
#define __CODE_WRITER__

#include <wx/string.h>
#include <wx/textctrl.h>

/** Abstracts the code generation from the target.
Because, in some cases the target is a file, sometimes a TextCtrl, and sometimes both.
*/
class CodeWriter
{
private:
	// Current indentation level in the file
	int m_indent;
	int m_cols;

protected:
	/// Write a wxString.
	virtual void DoWrite( const wxString& code ) = 0;

	/// Returns the size of the indentation - was useful when using spaces, now it is 1 because using tabs.
	virtual int GetIndentSize();

	/// Verifies that the wxString does not contain carraige return characters.
	bool StringOk( wxString s );

	/** Divides a badly formed string (including carriage returns) in simple
	columns, inserting them one after another and taking indent into account.
	*/
	void FixWrite( wxString s );

public:
	/// Constructor.
	CodeWriter();

	/// Destructor.
	virtual ~CodeWriter();

	/// Increment the indent.
	void Indent();

	/// Decrement the indent.
	void Unindent();

	/// Write a line of code.
	void WriteLn( wxString code = wxEmptyString );

	/// Writes a text string into the code.
	void Write( wxString code );

	/// Deletes all the code previously written.
	virtual void Clear() = 0;
};

//class wxScintilla;

class TCCodeWriter : public CodeWriter
{
private:
	wxTextCtrl *m_tc;

protected:
	void DoWrite( const wxString& code );

public:
	TCCodeWriter();
	TCCodeWriter( wxTextCtrl *tc );
	void SetTextCtrl( wxTextCtrl* tc );
	void Clear();
};

class StringCodeWriter : public CodeWriter
{
protected:
	wxString m_buffer;
	void DoWrite( const wxString& code );

public:
	StringCodeWriter();
	void Clear();
	wxString GetString();
};

class FileCodeWriter : public StringCodeWriter
{
private:
	wxString m_filename;
	bool m_useMicrosoftBOM;
	bool m_useUtf8;

protected:
	void WriteBuffer();

public:
	FileCodeWriter( const wxString &file, bool useMicrosoftBOM = false, bool useUtf8 = true );
	~FileCodeWriter();
	void Clear();
};

#endif //__CODE_WRITER__
