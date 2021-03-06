package org.netxms.ui.eclipse.objectview.actions;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkbenchWindowActionDelegate;
import org.eclipse.ui.PartInitException;
import org.netxms.ui.eclipse.objectview.views.PhysicalLinkView;
import org.netxms.ui.eclipse.tools.MessageDialogHelper;

/**
 * 
 * @author zev
 *
 */
public class ShowPhysicalLink implements IWorkbenchWindowActionDelegate
{
   private IWorkbenchWindow window;
   
   /* (non-Javadoc)
    * @see org.eclipse.ui.IWorkbenchWindowActionDelegate#dispose()
    */
   @Override
   public void dispose()
   {
   }

   /* (non-Javadoc)
    * @see org.eclipse.ui.IWorkbenchWindowActionDelegate#init(org.eclipse.ui.IWorkbenchWindow)
    */
   @Override
   public void init(IWorkbenchWindow window)
   {
      this.window = window;
   }

   /* (non-Javadoc)
    * @see org.eclipse.ui.IActionDelegate#run(org.eclipse.jface.action.IAction)
    */
   @Override
   public void run(IAction action)
   {
      if(window != null)
      {  
         try 
         {
            window.getActivePage().showView(PhysicalLinkView.ID,"0&0",IWorkbenchPage.VIEW_ACTIVATE);
         } 
         catch (PartInitException e) 
         {
            MessageDialogHelper.openError(window.getShell(), "Error openning Physical link view", String.format("Cannot open Physical link view: %s", e.getLocalizedMessage()));
         }
      }
   }

   /* (non-Javadoc)
    * @see org.eclipse.ui.IActionDelegate#selectionChanged(org.eclipse.jface.action.IAction, org.eclipse.jface.viewers.ISelection)
    */
   @Override
   public void selectionChanged(IAction action, ISelection selection)
   {
   }
}
