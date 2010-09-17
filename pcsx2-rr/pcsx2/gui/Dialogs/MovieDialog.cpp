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
	: wxDialogWithHelpers(parent, AddAppName(_("RePlay Movie")), pxDialogFlags().Resize().MinWidth( 230 ) )
{
	m_filepicker = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, L"Select a file" ,L"p2m.*",
		wxDefaultPosition, wxSize(250,25), wxFLP_DEFAULT_STYLE ,wxDefaultValidator, L"filepickerctrl"
	);
	*this	+=	Label(_("&File:"));
	*this	+=	m_filepicker;

	int bestHeight = GetBestSize().GetHeight();
	if( bestHeight < 200 ) bestHeight = 200;
	SetMinHeight( bestHeight );
	AddOkCancel();
}
