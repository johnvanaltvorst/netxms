/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2013 Victor Kirhenshtein
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
package org.netxms.ui.eclipse.objecttools;

import java.io.File;
import java.net.URLEncoder;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.action.ContributionItem;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.rap.rwt.widgets.ExternalBrowser;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.ui.ISources;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.menus.IWorkbenchContribution;
import org.eclipse.ui.services.IEvaluationService;
import org.eclipse.ui.services.IServiceLocator;
import org.netxms.client.NXCSession;
import org.netxms.client.events.Alarm;
import org.netxms.client.objects.AbstractNode;
import org.netxms.client.objects.AbstractObject;
import org.netxms.client.objects.Cluster;
import org.netxms.client.objects.Container;
import org.netxms.client.objects.ServiceRoot;
import org.netxms.client.objects.Subnet;
import org.netxms.client.objecttools.ObjectTool;
import org.netxms.ui.eclipse.jobs.ConsoleJob;
import org.netxms.ui.eclipse.objecttools.api.ObjectToolHandler;
import org.netxms.ui.eclipse.objecttools.views.FileViewer;
import org.netxms.ui.eclipse.objecttools.views.TableToolResults;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;
import org.netxms.ui.eclipse.tools.MessageDialogHelper;

/**
 * Dynamic object tools menu creator
 */
@SuppressWarnings("deprecation")
public class ObjectToolsDynamicMenu extends ContributionItem implements IWorkbenchContribution
{
	private IEvaluationService evalService;
	
	/**
	 * Creates a contribution item with a null id.
	 */
	public ObjectToolsDynamicMenu()
	{
		super();
	}

	/**
	 * Creates a contribution item with the given (optional) id.
	 * 
	 * @param id the contribution item identifier, or null
	 */
	public ObjectToolsDynamicMenu(String id)
	{
		super(id);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.menus.IWorkbenchContribution#initialize(org.eclipse.ui.services.IServiceLocator)
	 */
	@Override
	public void initialize(IServiceLocator serviceLocator)
	{
		evalService = (IEvaluationService)serviceLocator.getService(IEvaluationService.class);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.action.ContributionItem#fill(org.eclipse.swt.widgets.Menu, int)
	 */
	@Override
	public void fill(Menu menu, int index)
	{
		Object selection = evalService.getCurrentState().getVariable(ISources.ACTIVE_MENU_SELECTION_NAME);
		if ((selection == null) || !(selection instanceof IStructuredSelection))
			return;

		final Set<AbstractNode> nodes = buildNodeSet((IStructuredSelection)selection);
		final Menu toolsMenu = new Menu(menu);
		
		ObjectTool[] tools = ObjectToolsCache.getTools();
		Arrays.sort(tools, new Comparator<ObjectTool>() {
			@Override
			public int compare(ObjectTool arg0, ObjectTool arg1)
			{
				return arg0.getName().replace("&", "").compareToIgnoreCase(arg1.getName().replace("&", "")); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$
			}
		});
		
		Map<String, Menu> menus = new HashMap<String, Menu>();
		int added = 0;
		for(int i = 0; i < tools.length; i++)
		{
			boolean allowed = isToolAllowed(tools[i], nodes);
			
			if (allowed && isToolApplicable(tools[i], nodes))
			{
				String[] path = tools[i].getName().split("\\-\\>"); //$NON-NLS-1$
			
				Menu rootMenu = toolsMenu;
				for(int j = 0; j < path.length - 1; j++)
				{
					String key = path[j].replace("&", ""); //$NON-NLS-1$ //$NON-NLS-2$
					Menu currMenu = menus.get(key);
					if (currMenu == null)
					{
						currMenu = new Menu(rootMenu);
						MenuItem item = new MenuItem(rootMenu, SWT.CASCADE);
						item.setText(path[j]);
						item.setMenu(currMenu);
						menus.put(key, currMenu);
					}
					rootMenu = currMenu;
				}
				
				final MenuItem item = new MenuItem(rootMenu, SWT.PUSH);
				item.setText(path[path.length - 1]);
				item.setData(tools[i]);
				item.addSelectionListener(new SelectionAdapter() {
					@Override
					public void widgetSelected(SelectionEvent e)
					{
						executeObjectTool(nodes, (ObjectTool)item.getData());
					}
				});
				
				added++;
			}
		}

		if (added > 0)
		{
			MenuItem toolsMenuItem = new MenuItem(menu, SWT.CASCADE, index);
			toolsMenuItem.setText(Messages.get().ObjectToolsDynamicMenu_TopLevelLabel);
			toolsMenuItem.setMenu(toolsMenu);
		}
		else
		{
			toolsMenu.dispose();
		}
	}
	
	/**
	 * Build node set from selection
	 * 
	 * @param selection
	 * @return
	 */
	private Set<AbstractNode> buildNodeSet(IStructuredSelection selection)
	{
		final Set<AbstractNode> nodes = new HashSet<AbstractNode>();
		final NXCSession session = (NXCSession)ConsoleSharedData.getSession();
		
		for(Object o : selection.toList())
		{
			if (o instanceof AbstractNode)
			{
				nodes.add((AbstractNode)o);
			}
			else if ((o instanceof Container) || (o instanceof ServiceRoot) || (o instanceof Subnet) || (o instanceof Cluster))
			{
				for(AbstractObject n : ((AbstractObject)o).getAllChilds(AbstractObject.OBJECT_NODE))
					nodes.add((AbstractNode)n);
			}
			else if (o instanceof Alarm)
			{
				AbstractNode n = (AbstractNode)session.findObjectById(((Alarm)o).getSourceObjectId(), AbstractNode.class);
				if (n != null)
					nodes.add(n);
			}
		}
		return nodes;
	}
	
	/**
	 * Check if tool is allowed for execution on each node from set
	 * 
	 * @param tool
	 * @param nodes
	 * @return
	 */
	private static boolean isToolAllowed(ObjectTool tool, Set<AbstractNode> nodes)
	{
		if (tool.getType() != ObjectTool.TYPE_INTERNAL)
			return true;
		
		ObjectToolHandler handler = ObjectToolsCache.findHandler(tool.getData());
		if (handler != null)
		{
			for(AbstractNode n : nodes)
				if (!handler.canExecuteOnNode(n, tool))
					return false;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	/**
	 * Check if given tool is applicable for all nodes in set
	 * 
	 * @param tool
	 * @param nodes
	 * @return
	 */
	private static boolean isToolApplicable(ObjectTool tool, Set<AbstractNode> nodes)
	{
		for(AbstractNode n : nodes)
			if (!tool.isApplicableForNode(n))
				return false;
		return true;
	}
	
	/**
	 * Execute object tool on node set
	 * @param tool Object tool
	 */
	private void executeObjectTool(final Set<AbstractNode> nodes, final ObjectTool tool)
	{
		if ((tool.getFlags() & ObjectTool.ASK_CONFIRMATION) != 0)
		{
			String message = tool.getConfirmationText();
			if (nodes.size() == 1)
			{
				AbstractNode node = nodes.iterator().next();
				message = message.replace("%OBJECT_IP_ADDR%", node.getPrimaryIP().getHostAddress()); //$NON-NLS-1$
				message = message.replace("%OBJECT_NAME%", node.getObjectName()); //$NON-NLS-1$
				message = message.replace("%OBJECT_ID%", Long.toString(node.getObjectId())); //$NON-NLS-1$
			}
			else
			{
				message = message.replace("%OBJECT_IP_ADDR%", Messages.get().ObjectToolsDynamicMenu_MultipleNodes); //$NON-NLS-1$
				message = message.replace("%OBJECT_NAME%", Messages.get().ObjectToolsDynamicMenu_MultipleNodes); //$NON-NLS-1$
				message = message.replace("%OBJECT_ID%", Messages.get().ObjectToolsDynamicMenu_MultipleNodes); //$NON-NLS-1$
			}
			if (!MessageDialogHelper.openQuestion(PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell(), 
					Messages.get().ObjectToolsDynamicMenu_ConfirmExec, message))
				return;
		}
		
		for(AbstractNode n : nodes)
			executeObjectToolOnNode(n, tool);
	}
	
	/**
	 * Execute object tool on single node
	 * 
	 * @param node
	 * @param tool
	 */
	private void executeObjectToolOnNode(final AbstractNode node, final ObjectTool tool)
	{
		switch(tool.getType())
		{
			case ObjectTool.TYPE_INTERNAL:
				executeInternalTool(node, tool);
				break;
			case ObjectTool.TYPE_LOCAL_COMMAND:
				// local commands cannot be executed via browser
				//executeLocalCommand(node, tool);
				break;
			case ObjectTool.TYPE_SERVER_COMMAND:
				executeServerCommand(node, tool);
				break;
			case ObjectTool.TYPE_ACTION:
				executeAgentAction(node, tool);
				break;
			case ObjectTool.TYPE_TABLE_AGENT:
			case ObjectTool.TYPE_TABLE_SNMP:
				executeTableTool(node, tool);
				break;
			case ObjectTool.TYPE_URL:
				openURL(node, tool);
				break;
			case ObjectTool.TYPE_FILE_DOWNLOAD:
				executeFileDownload(node, tool);
				break;
		}
	}
	
	/**
	 * Execute table tool
	 * 
	 * @param node
	 * @param tool
	 */
	private void executeTableTool(final AbstractNode node, final ObjectTool tool)
	{
		final IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
		try
		{
			final IWorkbenchPage page = window.getActivePage();
			final TableToolResults view = (TableToolResults)page.showView(TableToolResults.ID,
					Long.toString(tool.getId()) + "&" + Long.toString(node.getObjectId()), IWorkbenchPage.VIEW_ACTIVATE); //$NON-NLS-1$
			view.refreshTable();
		}
		catch(PartInitException e)
		{
			MessageDialogHelper.openError(window.getShell(), Messages.get().ObjectToolsDynamicMenu_Error, String.format(Messages.get().ObjectToolsDynamicMenu_ErrorOpeningView, e.getLocalizedMessage()));
		}
	}

	/**
	 * @param node
	 * @param tool
	 */
	private void executeAgentAction(final AbstractNode node, final ObjectTool tool)
	{
		final NXCSession session = (NXCSession)ConsoleSharedData.getSession();
		final String action = substituteMacros(tool.getData(), node);
		new ConsoleJob(String.format(Messages.get().ObjectToolsDynamicMenu_ExecuteOnNode, node.getObjectName()), null, Activator.PLUGIN_ID, null) {
			@Override
			protected String getErrorMessage()
			{
				return String.format(Messages.get().ObjectToolsDynamicMenu_CannotExecuteOnNode, node.getObjectName());
			}

			@Override
			protected void runInternal(IProgressMonitor monitor) throws Exception
			{
				session.executeAction(node.getObjectId(), action);
				runInUIThread(new Runnable() {
					@Override
					public void run()
					{
						MessageDialogHelper.openInformation(null, Messages.get().ObjectToolsDynamicMenu_ToolExecution, String.format(Messages.get().ObjectToolsDynamicMenu_ExecSuccess, action, node.getObjectName()));
					}
				});
			}
		}.start();
	}

	/**
	 * Execute server command
	 * 
	 * @param node
	 * @param tool
	 */
	private void executeServerCommand(final AbstractNode node, final ObjectTool tool)
	{
		final NXCSession session = (NXCSession)ConsoleSharedData.getSession();
		new ConsoleJob(Messages.get().ObjectToolsDynamicMenu_ExecuteServerCmd, null, Activator.PLUGIN_ID, null) {
			@Override
			protected void runInternal(IProgressMonitor monitor) throws Exception
			{
				session.executeServerCommand(node.getObjectId(), tool.getData());
				runInUIThread(new Runnable() {
					@Override
					public void run()
					{
						MessageDialogHelper.openInformation(null, Messages.get().ObjectToolsDynamicMenu_Information, Messages.get().ObjectToolsDynamicMenu_ServerCommandExecuted);
					}
				});
			}
			
			@Override
			protected String getErrorMessage()
			{
				return Messages.get().ObjectToolsDynamicMenu_ServerCmdExecError;
			}
		}.start();
	}
	
	/**
	 * @param node
	 * @param tool
	 */
	private void executeFileDownload(final AbstractNode node, final ObjectTool tool)
	{
		final NXCSession session = (NXCSession)ConsoleSharedData.getSession();
		final String fileName = substituteMacros(tool.getData(), node);
		
		ConsoleJob job = new ConsoleJob(Messages.get().ObjectToolsDynamicMenu_DownloadFromAgent, null, Activator.PLUGIN_ID, null) {
			@Override
			protected String getErrorMessage()
			{
				return String.format(Messages.get().ObjectToolsDynamicMenu_DownloadError, fileName, node.getObjectName());
			}

			@Override
			protected void runInternal(IProgressMonitor monitor) throws Exception
			{
				final File file = session.downloadFileFromAgent(node.getObjectId(), fileName);
				runInUIThread(new Runnable() {
					@Override
					public void run()
					{
						final IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
						try
						{
							String secondaryId = Long.toString(node.getObjectId()) + "&" + URLEncoder.encode(fileName, "UTF-8"); //$NON-NLS-1$ //$NON-NLS-2$
							FileViewer view = (FileViewer)window.getActivePage().showView(FileViewer.ID, secondaryId, IWorkbenchPage.VIEW_ACTIVATE);
							view.showFile(file);
						}
						catch(Exception e)
						{
							MessageDialogHelper.openError(window.getShell(), Messages.get().ObjectToolsDynamicMenu_Error, String.format(Messages.get().ObjectToolsDynamicMenu_ErrorOpeningView, e.getLocalizedMessage()));
						}
					}
				});
			}
		};
		job.start();
	}

	/**
	 * @param node
	 * @param tool
	 */
	private void executeInternalTool(final AbstractNode node, final ObjectTool tool)
	{
		ObjectToolHandler handler = ObjectToolsCache.findHandler(tool.getData());
		if (handler != null)
		{
			handler.execute(node, tool);
		}
		else
		{
			MessageDialogHelper.openError(PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell(), Messages.get().ObjectToolsDynamicMenu_Error, Messages.get().ObjectToolsDynamicMenu_HandlerNotDefined);
		}
	}

	/**
	 * @param node
	 * @param tool
	 */
	private void openURL(final AbstractNode node, final ObjectTool tool)
	{
		final String url = substituteMacros(tool.getData(), node);
		
		final String sid = Long.toString(node.getObjectId()) + "&" + Long.toString(tool.getId());
		ExternalBrowser.open(sid, url, ExternalBrowser.LOCATION_BAR | ExternalBrowser.NAVIGATION_BAR | ExternalBrowser.STATUS);
	}
	
	/**
	 * Substitute macros in string
	 * 
	 * @param s
	 * @param node
	 * @return
	 */
	private static String substituteMacros(String s, AbstractNode node)
	{
		StringBuilder sb = new StringBuilder();
		
		char[] src = s.toCharArray();
		for(int i = 0; i < s.length(); i++)
		{
			if (src[i] == '%')
			{
				StringBuilder p = new StringBuilder();
				for(i++; src[i] != '%' && i < s.length(); i++)
					p.append(src[i]);
				if (p.length() == 0)		// %%
				{
					sb.append('%');
				}
				else
				{
					String name = p.toString();
					if (name.equals("OBJECT_IP_ADDR")) //$NON-NLS-1$
					{
						sb.append(node.getPrimaryIP().getHostAddress());
					}
					else if (name.equals("OBJECT_NAME")) //$NON-NLS-1$
					{
						sb.append(node.getObjectName());
					}
					else if (name.equals("OBJECT_ID")) //$NON-NLS-1$
					{
						sb.append(node.getObjectId());
					}
					else
					{
						String custAttr = node.getCustomAttributes().get(name);
						if (custAttr != null)
							sb.append(custAttr);
					}
				}
			}
			else
			{
				sb.append(src[i]);
			}
		}
		
		return sb.toString();
	}
}
