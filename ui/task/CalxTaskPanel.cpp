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

#include "calx/ui/task/CalxTaskPanel.h"
#include "calx/ctrl-lib/gcode/GCodeWriter.h"
#include "calx/ui/CalxApp.h"
#include "calx/ui/CalxErrorHandler.h"
#include "calx/ui/coord/CalxVirtualPlane.h"
#include "calx/ui/task/CalxGCodeTask.h"
#include <fstream>
#include <sstream>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include "calx/ui/coord/CalxPlaneList.h"
#include "calx/ctrl-lib/logger/Shortcuts.h"

namespace CalX::UI {

	wxDEFINE_EVENT(wxEVT_TASK_PANEL_ENABLE, wxThreadEvent);
	wxDEFINE_EVENT(wxEVT_TASK_PANEL_ATTACH_TASK, wxThreadEvent);
	wxDEFINE_EVENT(wxEVT_TASK_PANEL_REMOVE_TASK, wxThreadEvent);

	std::ostream &operator<<(std::ostream &out, const CalxTaskDescriptor &descr) {
		descr.dump(out);
		return out;
	}

	class CalxTaskAction : public CalxAction {
	 public:
		CalxTaskAction(CalxTaskPanel *panel, std::shared_ptr<CoordHandle> handle,
		               std::shared_ptr<CoordTask> task,
		               const CalxTaskDescriptor &descr, const TaskParameters &prms)
		    : descriptor(descr),
		      panel(panel),
		      handle(handle),
		      task(task),
		      prms(prms) {
			this->state = std::make_shared<TaskState>();
			this->state->plane = nullptr;
			this->state->work = false;
		}

		ErrorCode perform(SystemManager &sysman) override {
			Info(wxGetApp().getJournal())
			    << "Start execution of task (with speed " << this->prms.speed
			    << "): " << this->descriptor;
			TaskParameters parameters(this->prms);
			parameters.speed *= this->handle->getFloatPlane()->getSpeedScale();
			handle->open_session();
			panel->setEnabled(false);
			ErrorCode errcode =
			    task->perform(this->handle, parameters, sysman, this->state);
			wxGetApp().getErrorHandler()->handle(errcode);
			panel->setEnabled(true);
			handle->close_session();
			Info(wxGetApp().getJournal()) << "End of execution of task with errcode "
			                              << static_cast<int>(errcode);
			return errcode;
		}

		void stop() override {
			state->stop();
		}

	 private:
		const CalxTaskDescriptor &descriptor;
		CalxTaskPanel *panel;
		std::shared_ptr<CoordHandle> handle;
		std::shared_ptr<CoordTask> task;
		TaskParameters prms;
		std::shared_ptr<TaskState> state;
	};

	class CalxPreviewAction : public CalxAction {
	 public:
		CalxPreviewAction(CalxTaskPanel *panel, CalxVirtualPlaneDialog *dialog,
		                  std::shared_ptr<CoordTask> task, TaskParameters prms) {
			this->panel = panel;
			this->dialog = dialog;
			this->task = task;
			this->prms = prms;
			this->prms.speed *= dialog->getFloatPlane()->getSpeedScale();
			this->state = std::make_shared<TaskState>();
			this->state->plane = nullptr;
			this->state->work = false;
		}

		ErrorCode perform(SystemManager &sysman) override {
			dialog->setEnabled(false);
			panel->setEnabled(false);
			ErrorCode errcode =
			    task->perform(dialog->getPlane(), prms, sysman, state);
			wxGetApp().getErrorHandler()->handle(errcode);
			panel->setEnabled(true);
			dialog->setEnabled(true);
			dialog->Refresh();
			return errcode;
		}

		void stop() override {
			state->stop();
		}

	 private:
		CalxTaskPanel *panel;
		CalxVirtualPlaneDialog *dialog;
		std::shared_ptr<CoordTask> task;
		TaskParameters prms;
		std::shared_ptr<TaskState> state;
	};

	CalxTaskPanel::CalxTaskPanel(wxWindow *win, wxWindowID id)
	    : CalxPanelPane::CalxPanelPane(win, id) {
		std::string units = wxGetApp().getUnitProcessor().getUnits();
		this->queue = new CalxActionQueue(wxGetApp().getSystemManager(), [this]() {
			wxQueueEvent(this, new wxThreadEvent(wxEVT_COMMAND_QUEUE_UPDATE));
		});
		this->queue->start();
		wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
		SetSizer(sizer);
		wxSplitterWindow *splitter = new wxSplitterWindow(this, wxID_ANY);
		sizer->Add(splitter, 1, wxALL | wxEXPAND);

		this->taskPanel = new wxPanel(splitter, wxID_ANY);
		wxBoxSizer *taskSizer = new wxBoxSizer(wxVERTICAL);
		taskPanel->SetSizer(taskSizer);
		this->taskList = new wxListBox(taskPanel, wxID_ANY);
		taskSizer->Add(this->taskList, 1, wxALL | wxEXPAND);

		this->taskFactoryPanel = new wxPanel(taskPanel, wxID_ANY);
		taskSizer->Add(this->taskFactoryPanel, 0, wxALL | wxEXPAND);
		wxBoxSizer *taskFactorySizer = new wxBoxSizer(wxVERTICAL);
		taskFactoryPanel->SetSizer(taskFactorySizer);

		wxButton *removeButton = new wxButton(taskPanel, wxID_ANY, __("Remove"));
		taskSizer->Add(removeButton, 0, wxALL | wxEXPAND);
		removeButton->Bind(wxEVT_BUTTON, &CalxTaskPanel::OnRemoveClick, this);

		this->mainPanel = new wxPanel(splitter, wxID_ANY);
		wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
		mainPanel->SetSizer(mainSizer);

		this->execPanel = new wxPanel(this->mainPanel, wxID_ANY);
		mainSizer->Add(execPanel, 0, wxALL | wxEXPAND, 5);
		wxBoxSizer *execSizer = new wxBoxSizer(wxHORIZONTAL);
		execPanel->SetSizer(execSizer);
		wxButton *buildButton = new wxButton(execPanel, wxID_ANY, __("Build"));
		execSizer->Add(buildButton, 0, wxALL | wxALIGN_CENTER);
		this->plane = new wxChoice(execPanel, wxID_ANY);
		this->speed = new wxSpinCtrlDouble(
		    execPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
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
		execSizer->Add(new wxStaticText(execPanel, wxID_ANY, __("on")), 0,
		               wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);
		execSizer->Add(plane, 0, wxALL, 5);
		execSizer->Add(new wxStaticText(execPanel, wxID_ANY, __("with speed")), 0,
		               wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);
		execSizer->Add(speed, 0, wxALL, 5);
		execSizer->Add(
		    new wxStaticText(execPanel, wxID_ANY,
		                     wxGetApp().getUnitProcessor().getSpeedUnits()),
		    0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);
		this->stopButton = new wxButton(execPanel, wxID_ANY, __("Stop"));
		execSizer->Add(stopButton);
		wxButton *previewButton = new wxButton(execPanel, wxID_ANY, __("Preview"));
		execSizer->Add(previewButton);
		wxButton *linearizeButton =
		    new wxButton(execPanel, wxID_ANY, __("Linearize to GCode"));
		execSizer->Add(linearizeButton);
		wxButton *moveToStartButton =
		    new wxButton(execPanel, wxID_ANY, __("Move to start"));
		execSizer->Add(moveToStartButton);
		buildButton->Bind(wxEVT_BUTTON, &CalxTaskPanel::OnBuildClick, this);
		stopButton->Bind(wxEVT_BUTTON, &CalxTaskPanel::OnStopClick, this);
		previewButton->Bind(wxEVT_BUTTON, &CalxTaskPanel::OnPreviewClick, this);
		linearizeButton->Bind(wxEVT_BUTTON, &CalxTaskPanel::OnLinearizeClick, this);
		moveToStartButton->Bind(wxEVT_BUTTON, &CalxTaskPanel::OnMoveToStartClick,
		                        this);

		splitter->Initialize(mainPanel);
		splitter->SplitVertically(taskPanel, mainPanel);
		splitter->SetSashGravity(0.1f);

		Layout();
		setEnabled(true);
		this->SetScrollRate(5, 5);

		taskList->Bind(wxEVT_LISTBOX, &CalxTaskPanel::OnListClick, this);
		this->Bind(wxEVT_CLOSE_WINDOW, &CalxTaskPanel::OnExit, this);
		this->Bind(wxEVT_TASK_PANEL_ENABLE, &CalxTaskPanel::OnEnableEvent, this);
		this->Bind(wxEVT_TASK_PANEL_ATTACH_TASK, &CalxTaskPanel::OnAttachTask,
		           this);
		this->Bind(wxEVT_TASK_PANEL_REMOVE_TASK, &CalxTaskPanel::OnRemoveTask,
		           this);
	}

	void CalxTaskPanel::attachTaskFactory(const std::string &name,
	                                      CalxTaskFactory *factory) {
		wxButton *factButton = new wxButton(
		    this->taskFactoryPanel, wxID_ANY,
		    __("New") + std::string(" ") + name + std::string(" ") + __("task"));
		this->taskFactoryPanel->GetSizer()->Add(factButton, 0, wxALL | wxEXPAND);
		factButton->Bind(wxEVT_BUTTON, &CalxTaskPanel::OnNewTaskClick, this);
		this->factories[factButton] = factory;
	}

	void CalxTaskPanel::shutdown() {
		this->queue->stop();
	}

	bool CalxTaskPanel::isBusy() {
		return queue->isBusy();
	}

	void CalxTaskPanel::stop() {
		this->queue->stopCurrent();
	}

	std::size_t CalxTaskPanel::getTaskCount() {
		return this->list.size();
	}

	void CalxTaskPanel::removeTask(std::size_t idx) {
		wxThreadEvent evt(wxEVT_TASK_PANEL_REMOVE_TASK);
		evt.SetPayload(idx);
		wxPostEvent(this, evt);
	}

	void CalxTaskPanel::attachTask(const std::string &title,
	                               std::shared_ptr<CalxTaskFactory> factory) {
		wxThreadEvent evt(wxEVT_TASK_PANEL_ATTACH_TASK);
		evt.SetPayload(std::make_pair(title, factory));
		wxPostEvent(this, evt);
	}

	void CalxTaskPanel::updateUI() {
		for (const auto &t : this->list) {
			t->Show(false);
		}
		if (taskList->GetSelection() != wxNOT_FOUND) {
			this->list.at((std::size_t) taskList->GetSelection())->Show(true);
		}
		plane->Clear();
		for (std::size_t i = 0;
		     i < wxGetApp().getSystemManager().getCoordPlaneSet().getCoordCount();
		     i++) {
			if (std::shared_ptr<CoordHandle> handle = wxGetApp()
			                                              .getSystemManager()
			                                              .getCoordPlaneSet()
			                                              .getCoord(i)
			                                              .lock()) {
				if (!handle->isSessionOpened()) {
					plane->Append("Plane #" + std::to_string(handle->getID()));
				}
			}
		}
		if (wxGetApp().getSystemManager().getCoordPlaneSet().getCoordCount() > 0) {
			plane->SetSelection(0);
		}
		this->mainPanel->Layout();
		Layout();
	}

	void CalxTaskPanel::setEnabled(bool e) {
		wxThreadEvent evt(wxEVT_TASK_PANEL_ENABLE);
		evt.SetPayload(e);
		wxPostEvent(this, evt);
	}

	void CalxTaskPanel::attachTaskImpl(const std::string &title,
	                                   CalxTaskFactory &factory) {
		CalxTaskHandle *task = factory.newTask(this->mainPanel);
		if (task != nullptr) {
			list.push_back(task);
			taskList->Append(title.empty() ? task->getName() : title);
			mainPanel->GetSizer()->Add(task, 1, wxALL | wxEXPAND, 5);
			taskList->SetSelection((int) list.size() - 1);
			Layout();
			updateUI();
		}
	}

	void CalxTaskPanel::removeTaskImpl(std::size_t idx) {
		if (idx < this->list.size()) {
			taskList->Delete(static_cast<unsigned int>(idx));
			this->list.at(idx)->Close(true);
			this->list.erase(this->list.begin() + idx);
			updateUI();
		}
	}

	void CalxTaskPanel::OnAttachTask(wxThreadEvent &evt) {
		using Payload = std::pair<std::string, std::shared_ptr<CalxTaskFactory>>;
		Payload taskInfo = evt.GetPayload<Payload>();
		this->attachTaskImpl(taskInfo.first, *taskInfo.second);
	}

	void CalxTaskPanel::OnRemoveTask(wxThreadEvent &evt) {
		std::size_t idx = evt.GetPayload<std::size_t>();
		this->removeTaskImpl(idx);
	}

	void CalxTaskPanel::OnExit(wxCloseEvent &evt) {
		for (const auto &kv : this->factories) {
			delete kv.second;
		}
		for (const auto &h : list) {
			h->Close(true);
		}
		Destroy();
	}

	void CalxTaskPanel::OnNewTaskClick(wxCommandEvent &evt) {
		if (this->factories.count(evt.GetEventObject()) != 0) {
			CalxTaskFactory *fact = this->factories[evt.GetEventObject()];
			this->attachTaskImpl("", *fact);
		}
	}

	void CalxTaskPanel::OnRemoveClick(wxCommandEvent &evt) {
		if (taskList->GetSelection() != wxNOT_FOUND) {
			std::size_t sel = (std::size_t) taskList->GetSelection();
			this->removeTaskImpl(sel);
		} else {
			std::string message = __("Select task to remove");
			wxMessageDialog *dialog = new wxMessageDialog(
			    this, message, __("Warning"), wxOK | wxICON_WARNING);
			dialog->ShowModal();
			dialog->Destroy();
		}
	}

	void CalxTaskPanel::OnListClick(wxCommandEvent &evt) {
		updateUI();
	}

	void CalxTaskPanel::OnBuildClick(wxCommandEvent &evt) {
		if (taskList->GetSelection() != wxNOT_FOUND &&
		    plane->GetSelection() != wxNOT_FOUND) {
			list.at((std::size_t) taskList->GetSelection())->update();
			std::shared_ptr<CoordTask> task =
			    list.at((std::size_t) taskList->GetSelection())->getTask();
			if (std::shared_ptr<CoordHandle> handle =
			        wxGetApp()
			            .getSystemManager()
			            .getCoordPlaneSet()
			            .getCoord((std::size_t) plane->GetSelection())
			            .lock()) {
				float speed = this->speed->GetValue();
				TaskParameters prms = { (float) speed };
				queue->addAction(std::make_unique<CalxTaskAction>(
				    this, handle, task,
				    *list.at((std::size_t) taskList->GetSelection()), prms));
			}
		} else {
			std::string message = __("Select coordinate plane");
			if (taskList->GetSelection() == wxNOT_FOUND) {
				message = __("Select task to build");
			}
			wxMessageDialog *dialog = new wxMessageDialog(
			    this, message, __("Warning"), wxOK | wxICON_WARNING);
			dialog->ShowModal();
			dialog->Destroy();
		}
	}

	void CalxTaskPanel::OnPreviewClick(wxCommandEvent &evt) {
		if (taskList->GetSelection() != wxNOT_FOUND &&
		    plane->GetSelection() != wxNOT_FOUND) {
			list.at((std::size_t) taskList->GetSelection())->update();
			std::shared_ptr<CoordTask> task =
			    list.at((std::size_t) taskList->GetSelection())->getTask();
			if (std::shared_ptr<CoordHandle> handle =
			        wxGetApp()
			            .getSystemManager()
			            .getCoordPlaneSet()
			            .getCoord((std::size_t) plane->GetSelection())
			            .lock()) {
				if (!handle->isMeasured()) {
					wxMessageBox(__("Plane need to be measured before preview"),
					             __("Warning"), wxICON_WARNING);
					return;
				}
				TaskParameters prms = { (float) this->speed->GetValue() };
				CalxVirtualPlaneDialog *dialog = new CalxVirtualPlaneDialog(
				    this, wxID_ANY, handle, wxSize(500, 500));
				queue->addAction(
				    std::make_unique<CalxPreviewAction>(this, dialog, task, prms));
				dialog->ShowModal();
				delete dialog;
			}
		} else {
			std::string message = __("Select coordinate plane");
			if (taskList->GetSelection() == wxNOT_FOUND) {
				message = __("Select task to build");
			}
			wxMessageDialog *dialog = new wxMessageDialog(
			    this, message, __("Warning"), wxOK | wxICON_WARNING);
			dialog->ShowModal();
			dialog->Destroy();
		}
	}

	void CalxTaskPanel::OnLinearizeClick(wxCommandEvent &evt) {
		if (taskList->GetSelection() != wxNOT_FOUND &&
		    plane->GetSelection() != wxNOT_FOUND) {
			list.at((std::size_t) taskList->GetSelection())->update();
			std::shared_ptr<CoordTask> task =
			    list.at((std::size_t) taskList->GetSelection())->getTask();
			if (std::shared_ptr<CoordHandle> handle =
			        wxGetApp()
			            .getSystemManager()
			            .getCoordPlaneSet()
			            .getCoord((std::size_t) plane->GetSelection())
			            .lock()) {
				if (!handle->isMeasured()) {
					wxMessageBox(__("Plane need to be measured to linearize"),
					             __("Warning"), wxICON_WARNING);
					return;
				}
				TaskParameters prms = { (float) this->speed->GetValue() };

				std::stringstream ss;
				std::shared_ptr<TaskState> state = std::make_shared<TaskState>();
				std::shared_ptr<GCodeWriter> writer = std::make_shared<GCodeWriter>(
				    handle->getPosition(), handle->getSize(),
				    list.at((std::size_t) taskList->GetSelection())->getTranslator(),
				    ss);
				this->setEnabled(false);
				wxGetApp().getErrorHandler()->handle(
				    task->perform(writer, prms, wxGetApp().getSystemManager(), state));
				this->setEnabled(true);
				writer->close();

				wxFileDialog *dialog =
				    new wxFileDialog(this, __("Export linearized GCode"), "", "", "",
				                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				if (dialog->ShowModal() == wxID_OK) {
					std::string path = dialog->GetPath().ToStdString();
					std::ofstream out(path);
					out << ss.str();
					out.close();
				}
				ss.seekg(0);

				std::unique_ptr<CoordTranslator> translator =
				    list.at((std::size_t) taskList->GetSelection())
				        ->getTranslator()
				        ->clone(nullptr);
				std::shared_ptr<ComplexCoordTranslator> trans = nullptr;
				if (translator->getType() == CoordType::ComplexCoord) {
					std::shared_ptr<CoordTranslator> shtrans = std::move(translator);
					trans = std::static_pointer_cast<ComplexCoordTranslator>(shtrans);
				} else {
					std::shared_ptr<CoordTranslator> shtrans = std::move(translator);
					trans = std::make_shared<ComplexCoordTranslator>(shtrans);
				}
				CalxGcodeHandle *gcodeHandle = new CalxGcodeHandle(
				    mainPanel, wxID_ANY,
				    __("Linear ") + taskList->GetStringSelection().ToStdString(), ss,
				    trans);

				list.push_back(gcodeHandle);
				taskList->Append(__("Linear ") +
				                 taskList->GetStringSelection().ToStdString());
				mainPanel->GetSizer()->Add(gcodeHandle, 1, wxALL | wxEXPAND, 5);
				taskList->SetSelection((int) list.size() - 1);
				Layout();
				updateUI();
				dialog->Destroy();
			}
		} else {
			std::string message = __("Select coordinate plane");
			if (taskList->GetSelection() == wxNOT_FOUND) {
				message = __("Select task to build");
			}
			wxMessageDialog *dialog = new wxMessageDialog(
			    this, message, __("Warning"), wxOK | wxICON_WARNING);
			dialog->ShowModal();
			dialog->Destroy();
		}
	}

	void CalxTaskPanel::OnStopClick(wxCommandEvent &evt) {
		this->queue->stopCurrent();
	}

	void CalxTaskPanel::OnEnableEvent(wxThreadEvent &evt) {
		bool e = evt.GetPayload<bool>();
		for (const auto &h : list) {
			h->Enable(e);
		}
		taskPanel->Enable(e);
		for (auto i = execPanel->GetChildren().begin();
		     i != execPanel->GetChildren().end(); ++i) {
			if (*i != this->stopButton) {
				(*i)->Enable(e);
			}
		}
		this->stopButton->Enable(!e);
	}

	void CalxTaskPanel::OnMoveToStartClick(wxCommandEvent &evt) {
		if (taskList->GetSelection() != wxNOT_FOUND &&
		    plane->GetSelection() != wxNOT_FOUND) {
			list.at((std::size_t) taskList->GetSelection())->update();
			std::shared_ptr<CoordTask> task =
			    list.at((std::size_t) taskList->GetSelection())->getTask();
			if (std::shared_ptr<CoordHandle> handle =
			        wxGetApp()
			            .getSystemManager()
			            .getCoordPlaneSet()
			            .getCoord((std::size_t) plane->GetSelection())
			            .lock()) {
				std::optional<motor_point_t> start =
				    task->getStartPoint(handle->getPosition(), handle->getSize(),
				                        wxGetApp().getSystemManager());
				if (start.has_value()) {
					float scale = wxGetApp()
					                  .getSystemManager()
					                  .getConfiguration()
					                  .getEntry(CalxConfiguration::Units)
					                  ->getReal(CalxUnitConfiguration::UnitScale, 1.0f);
					float unit_speed =
					    wxGetApp()
					        .getSystemManager()
					        .getConfiguration()
					        .getEntry(CalxConfiguration::Units)
					        ->getReal(CalxUnitConfiguration::UnitSpeed, 4000.0);
					coord_point_t dest = { start.value().x / scale,
						                     start.value().y / scale };
					wxGetApp()
					    .getMainFrame()
					    ->getPlaneList()
					    ->getPlaneHandle((std::size_t) plane->GetSelection())
					    ->getController()
					    ->move(dest, unit_speed, false, false);
				}
			}
		} else {
			std::string message = __("Select coordinate plane");
			if (taskList->GetSelection() == wxNOT_FOUND) {
				message = __("Select task to calculate");
			}
			wxMessageDialog *dialog = new wxMessageDialog(
			    this, message, __("Warning"), wxOK | wxICON_WARNING);
			dialog->ShowModal();
			dialog->Destroy();
		}
	}
}  // namespace CalX::UI
