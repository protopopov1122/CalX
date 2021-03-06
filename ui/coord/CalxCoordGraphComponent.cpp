/*
        SPDX-License-Identifier: LGPL-3.0

        Copyright (c) 2017-2019 Jevgenijs Protopopovs

        This file is part of CalX project.

        CalX is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License version 3 as published
   by
        the Free Software Foundation.


        CalX is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with CalX.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "calx/ui/coord/CalxCoordGraphComponent.h"
#include "calx/ctrl-lib/graph/MathEngine.h"
#include "calx/ctrl-lib/graph/FunctionParser.h"
#include "calx/ctrl-lib/misc/GraphBuilder.h"
#include "calx/ui/CalxErrorHandler.h"
#include <sstream>

namespace CalX::UI {

	CalxCoordComponent *CalxCoordGraphComponentFactory::newComponent(
	    wxWindow *win, CalxCoordController *ctrl) {
		return new CalxCoordGraphComponent(win, wxID_ANY, ctrl);
	}

	CalxCoordGraphComponent::CalxCoordGraphComponent(
	    wxWindow *win, wxWindowID id, CalxCoordController *controller)
	    : CalxCoordComponent::CalxCoordComponent(win, id),
	      controller(controller) {
		std::string units = wxGetApp().getUnitProcessor().getUnits();
		ConfiguationFlatDictionary *graphconf =
		    wxGetApp().getSystemManager().getConfiguration().getEntry(
		        CalxConfiguration::Graph);
		wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
		SetSizer(sizer);

		wxPanel *graphPanel = new wxPanel(this, wxID_ANY);
		sizer->Add(graphPanel, 0, wxRIGHT, 10);
		wxFlexGridSizer *graphSizer = new wxFlexGridSizer(3);
		graphPanel->SetSizer(graphSizer);

		this->expr = new wxTextCtrl(graphPanel, wxID_ANY, "x");
		this->xmin = new wxSpinCtrlDouble(
		    graphPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		    wxSP_ARROW_KEYS, INT_MIN, INT_MAX,
		    graphconf->getReal(CalxGraphConfiguration::XFrom, -10.0),
		    graphconf->getReal(CalxGraphConfiguration::XAxisStep, 0.001));
		this->xmax = new wxSpinCtrlDouble(
		    graphPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		    wxSP_ARROW_KEYS, INT_MIN, INT_MAX,
		    graphconf->getReal(CalxGraphConfiguration::XTo, 10.0),
		    graphconf->getReal(CalxGraphConfiguration::XAxisStep, 0.001));
		this->ymin = new wxSpinCtrlDouble(
		    graphPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		    wxSP_ARROW_KEYS, INT_MIN, INT_MAX,
		    graphconf->getReal(CalxGraphConfiguration::YFrom, -10.0),
		    graphconf->getReal(CalxGraphConfiguration::YAxisStep, 0.001));
		this->ymax = new wxSpinCtrlDouble(
		    graphPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		    wxSP_ARROW_KEYS, INT_MIN, INT_MAX,
		    graphconf->getReal(CalxGraphConfiguration::YTo, 10.0),
		    graphconf->getReal(CalxGraphConfiguration::YAxisStep, 0.001));
		this->step = new wxSpinCtrlDouble(
		    graphPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		    wxSP_ARROW_KEYS,
		    graphconf->getReal(CalxGraphConfiguration::StepStep, 0.001), INT_MAX,
		    graphconf->getReal(CalxGraphConfiguration::Step, 0.1),
		    graphconf->getReal(CalxGraphConfiguration::StepStep, 0.001));
		this->speed = new wxSpinCtrlDouble(
		    graphPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		    wxSP_ARROW_KEYS, 0,
		    wxGetApp()
		        .getSystemManager()
		        .getConfiguration()
		        .getEntry(CalxConfiguration::Units)
		        ->getReal(CalxUnitConfiguration::UnitSpeed, 4000.0),
		    wxGetApp()
		        .getSystemManager()
		        .getConfiguration()
		        .getEntry(CalxConfiguration::Units)
		        ->getReal(CalxUnitConfiguration::UnitSpeed, 4000.0),
		    wxGetApp().getUnitProcessor().getSpeedPrecision());
		wxButton *buildButton = new wxButton(graphPanel, wxID_ANY, __("Build"));
		wxButton *previewButton = new wxButton(graphPanel, wxID_ANY, __("Preview"));

		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, __("Function ")), 0,
		                wxALIGN_RIGHT | wxRIGHT, 10);
		graphSizer->Add(expr, 0, wxEXPAND);
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY,
		                                 __("X axis range") + std::string(":")));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, __("min")), 0,
		                wxALIGN_RIGHT | wxRIGHT, 10);
		graphSizer->Add(xmin, 0, wxEXPAND);
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, __("max")), 0,
		                wxALIGN_RIGHT | wxRIGHT, 10);
		graphSizer->Add(xmax, 0, wxEXPAND);
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY,
		                                 __("Y axis range") + std::string(":")));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, __("min")), 0,
		                wxALIGN_RIGHT | wxRIGHT, 10);
		graphSizer->Add(ymin, 0, wxEXPAND);
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, __("max")), 0,
		                wxALIGN_RIGHT | wxRIGHT, 10);
		graphSizer->Add(ymax, 0, wxEXPAND);
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, __("X axis step")),
		                0, wxALIGN_RIGHT | wxRIGHT, 10);
		graphSizer->Add(step, 0, wxEXPAND);
		graphSizer->Add(new wxStaticText(this, wxID_ANY, ""));
		graphSizer->Add(new wxStaticText(graphPanel, wxID_ANY, __("Build speed")),
		                0, wxALIGN_RIGHT | wxRIGHT, 10);
		graphSizer->Add(speed, 0, wxEXPAND);
		graphSizer->Add(new wxStaticText(
		    this, wxID_ANY, wxGetApp().getUnitProcessor().getSpeedUnits()));
		graphSizer->Add(buildButton);
		graphSizer->Add(previewButton);

		ConfigurationCatalogue &conf =
		    wxGetApp().getSystemManager().getConfiguration();
		coord_point_t cen = {
			static_cast<double>(conf.getEntry(CalxConfiguration::Coordinates)
			                        ->getReal(CalxCoordConfiguration::OffsetX, 0.0)),
			static_cast<double>(conf.getEntry(CalxConfiguration::Coordinates)
			                        ->getReal(CalxCoordConfiguration::OffsetY, 0.0))
		};
		coord_scale_t scl = {
			static_cast<double>(conf.getEntry(CalxConfiguration::Coordinates)
			                        ->getReal(CalxCoordConfiguration::ScaleX, 1.0)),
			static_cast<double>(conf.getEntry(CalxConfiguration::Coordinates)
			                        ->getReal(CalxCoordConfiguration::ScaleY, 1.0))
		};
		std::shared_ptr<LinearCoordTranslator> basic =
		    std::make_shared<LinearCoordTranslator>(cen, scl);
		this->trans = std::make_shared<ComplexCoordTranslator>(basic);
		this->translator = new CalxCoordFilterCtrl(this, wxID_ANY, this->trans);
		sizer->Add(this->translator, 1, wxALL | wxEXPAND, 5);

		buildButton->Bind(wxEVT_BUTTON, &CalxCoordGraphComponent::OnBuildClick,
		                  this);
		previewButton->Bind(wxEVT_BUTTON, &CalxCoordGraphComponent::OnPreviewClick,
		                    this);
		Bind(wxEVT_CLOSE_WINDOW, &CalxCoordGraphComponent::OnClose, this);
	}

	void CalxCoordGraphComponent::OnClose(wxCloseEvent &evt) {}

	void CalxCoordGraphComponent::OnBuildClick(wxCommandEvent &evt) {
		std::stringstream ss(this->expr->GetValue().ToStdString());
		FunctionParser parser(std::make_unique<FunctionLexer>(ss));
		std::unique_ptr<Node> node = parser.parse();
		if (node == nullptr) {
			wxGetApp().getErrorHandler()->handle(ErrorCode::MathExprError);
			return;
		}
		double minx = xmin->GetValue();
		double maxx = xmax->GetValue();
		double miny = ymin->GetValue();
		double maxy = ymax->GetValue();
		double step = this->step->GetValue();
		double speed = this->speed->GetValue();
		coord_point_t min = { minx, miny };
		coord_point_t max = { maxx, maxy };
		std::unique_ptr<GraphBuilder> graph =
		    std::make_unique<GraphBuilder>(std::move(node), min, max, step);
		this->controller->build(this->translator->getTranslator(), std::move(graph),
		                        speed);
	}

	void CalxCoordGraphComponent::OnPreviewClick(wxCommandEvent &evt) {
		if (!this->controller->getHandle()->isMeasured()) {
			wxMessageBox(__("Plane need to be measured before preview"),
			             __("Warning"), wxICON_WARNING);
			return;
		}
		std::stringstream ss(this->expr->GetValue().ToStdString());
		FunctionParser parser(std::make_unique<FunctionLexer>(ss));
		std::unique_ptr<Node> node = parser.parse();
		if (node == nullptr) {
			wxGetApp().getErrorHandler()->handle(ErrorCode::MathExprError);
			return;
		}
		double minx = xmin->GetValue();
		double maxx = xmax->GetValue();
		double miny = ymin->GetValue();
		double maxy = ymax->GetValue();
		double step = this->step->GetValue();
		double speed = this->speed->GetValue();
		coord_point_t min = { minx, miny };
		coord_point_t max = { maxx, maxy };
		std::unique_ptr<GraphBuilder> graph =
		    std::make_unique<GraphBuilder>(std::move(node), min, max, step);
		CalxVirtualPlaneDialog *dialog = new CalxVirtualPlaneDialog(
		    this, wxID_ANY, this->controller->getHandle(), wxSize(500, 500));

		this->controller->preview(dialog, this->translator->getTranslator(),
		                          std::move(graph), speed);
		dialog->ShowModal();
		delete dialog;
	}
}  // namespace CalX::UI
