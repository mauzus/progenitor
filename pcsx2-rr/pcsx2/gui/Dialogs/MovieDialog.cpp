/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "App.h"
#include "AppCommon.h"

#include "Dialogs/ModalPopups.h"

//#include "Resources/EmbeddedImage.h"
//#include "Resources/Dualshock.h"

#include <wx/mstream.h>
#include <wx/hyperlink.h>
#include <wx/stdpaths.h>
#include <wx/file.h>
#include <wx/dir.h>
#include <wx/filepicker.h>

using namespace pxSizerFlags;

// --------------------------------------------------------------------------------------
//  AboutBoxDialog  Implementation
// --------------------------------------------------------------------------------------

Dialogs::PlayMovieDialog::PlayMovieDialog( wxWindow* parent )
	: wxDialogWithHelpers(parent, AddAppName(_("RePlay Movie")) )
{
	SetMinWidth( 480 );

	wxStaticBoxSizer& groupSizer = *new wxStaticBoxSizer( wxVERTICAL, this, _("Option") );


	struct CheckTextMessage
	{
		wxString label, tooltip;
	};

	const CheckTextMessage check_text[2] =
	{
		{
			_("Start Paused"),
			wxEmptyString
		},
		{
			_("Read Only"),
			wxEmptyString
		}
	};

	for( int i=0; i<2; ++i )
	{
		groupSizer += (m_checkbox[i] = new pxCheckBox( this, check_text[i].label ));
		m_checkbox[i]->SetToolTip( check_text[i].tooltip );
	}

	wxStaticBoxSizer& fileinfo	 = *new wxStaticBoxSizer( wxVERTICAL, this, _("FileInfo") );
	fileinfo	+= Label(_("&Frame:"))| StdExpand();
	fileinfo	+= Label(_("&ReRecordCount:"))| StdExpand();

	wxBoxSizer& boxsizer	 = *new wxBoxSizer( wxHORIZONTAL );
	boxsizer += groupSizer;
	boxsizer += fileinfo;

	m_filepicker = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, L"Select a file" ,L"*.p2m",
		wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE ,wxDefaultValidator, L"filepickerctrl"
	);
	*this	+=	Label(_("&File:"));
	*this	+=	m_filepicker | StdExpand();
	*this	+=	boxsizer	| StdExpand();

	AddOkCancel();
}
