/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2011 Victor Kirhenshtein
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
package org.netxms.ui.eclipse.serverconfig.dialogs;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.netxms.client.InetAddressListElement;
import org.netxms.ui.eclipse.objectbrowser.api.ObjectSelectionFilterFactory;
import org.netxms.ui.eclipse.objectbrowser.widgets.ObjectSelector;
import org.netxms.ui.eclipse.objectbrowser.widgets.ZoneSelector;
import org.netxms.ui.eclipse.serverconfig.Messages;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;
import org.netxms.ui.eclipse.tools.MessageDialogHelper;
import org.netxms.ui.eclipse.tools.WidgetHelper;
import org.netxms.ui.eclipse.widgets.LabeledText;

/**
 * Dialog for adding address list element
 */
public class AddAddressListElementDialog extends Dialog
{
	private Button radioSubnet;
	private Button radioRange;
	private LabeledText textAddr1;
	private LabeledText textAddr2;
   private LabeledText comment;
	private ZoneSelector zoneSelector;
	private ObjectSelector proxySelector;
	private InetAddressListElement element;
	private boolean enableProxySelection;
	
	/**
	 * @param parentShell
	 * @param inetAddressListElement 
	 */
	public AddAddressListElementDialog(Shell parentShell, boolean enableProxySelection, InetAddressListElement inetAddressListElement)
	{
		super(parentShell);
		this.enableProxySelection = enableProxySelection;
		element = inetAddressListElement;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.window.Window#configureShell(org.eclipse.swt.widgets.Shell)
	 */
	@Override
	protected void configureShell(Shell newShell)
	{
		super.configureShell(newShell);
		newShell.setText(Messages.get().AddAddressListElementDialog_Title);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.dialogs.Dialog#createDialogArea(org.eclipse.swt.widgets.Composite)
	 */
	@Override
	protected Control createDialogArea(Composite parent)
	{
		Composite dialogArea = (Composite)super.createDialogArea(parent);
		
		GridLayout layout = new GridLayout();
		layout.marginHeight = WidgetHelper.DIALOG_HEIGHT_MARGIN;
		layout.marginWidth = WidgetHelper.DIALOG_WIDTH_MARGIN;
		layout.verticalSpacing = WidgetHelper.DIALOG_SPACING;
		dialogArea.setLayout(layout);
		
		radioSubnet = new Button(dialogArea, SWT.RADIO);
		radioSubnet.setText(Messages.get().AddAddressListElementDialog_Subnet);
		radioSubnet.setSelection(true);
		radioSubnet.addSelectionListener(new SelectionListener() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				textAddr1.setLabel(Messages.get().AddAddressListElementDialog_NetAddr);
				textAddr2.setLabel(Messages.get().AddAddressListElementDialog_NetMask);
			}
			
			@Override
			public void widgetDefaultSelected(SelectionEvent e)
			{
				widgetSelected(e);
			}
		});
		
		radioRange = new Button(dialogArea, SWT.RADIO);
		radioRange.setText(Messages.get().AddAddressListElementDialog_AddrRange);
		radioRange.addSelectionListener(new SelectionListener() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				textAddr1.setLabel(Messages.get().AddAddressListElementDialog_StartAddr);
				textAddr2.setLabel(Messages.get().AddAddressListElementDialog_EndAddr);
			}
			
			@Override
			public void widgetDefaultSelected(SelectionEvent e)
			{
				widgetSelected(e);
			}
		});
		
		textAddr1 = new LabeledText(dialogArea, SWT.NONE);
		textAddr1.setLabel(Messages.get().AddAddressListElementDialog_NetAddr);
		textAddr1.setText("0.0.0.0"); //$NON-NLS-1$
		GridData gd = new GridData();
		gd.horizontalAlignment = SWT.FILL;
		gd.grabExcessHorizontalSpace = true;
		gd.widthHint = 300;
		textAddr1.setLayoutData(gd);
		
		textAddr2 = new LabeledText(dialogArea, SWT.NONE);
		textAddr2.setLabel(Messages.get().AddAddressListElementDialog_NetMask);
		textAddr2.setText("0"); //$NON-NLS-1$
		gd = new GridData();
		gd.horizontalAlignment = SWT.FILL;
		gd.grabExcessHorizontalSpace = true;
		textAddr2.setLayoutData(gd);
		
		if (enableProxySelection)
		{
		   if (ConsoleSharedData.getSession().isZoningEnabled())
		   {
		      zoneSelector = new ZoneSelector(dialogArea, SWT.NONE, true);
		      zoneSelector.setLabel("Zone");
		      gd = new GridData();
		      gd.horizontalAlignment = SWT.FILL;
		      gd.grabExcessHorizontalSpace = true;
		      zoneSelector.setLayoutData(gd);
		   }
		   
		   proxySelector = new ObjectSelector(dialogArea, SWT.NONE, true);
		   proxySelector.setLabel("Proxy node");
		   proxySelector.setClassFilter(ObjectSelectionFilterFactory.getInstance().createNodeSelectionFilter(false));
         gd = new GridData();
         gd.horizontalAlignment = SWT.FILL;
         gd.grabExcessHorizontalSpace = true;
         proxySelector.setLayoutData(gd);
		}
		
      comment = new LabeledText(dialogArea, SWT.NONE);
      comment.setLabel("Comment");
      comment.getTextControl().setTextLimit(255);
      gd = new GridData();
      gd.horizontalAlignment = SWT.FILL;
      gd.grabExcessHorizontalSpace = true;
      comment.setLayoutData(gd);
      
      if (element != null)
      {
         radioSubnet.setSelection(element.isSubnet());
         radioRange.setSelection(!element.isSubnet());
         textAddr1.setText(element.getBaseAddress().getHostAddress());
         textAddr2.setText(element.isSubnet() ? Integer.toString(element.getMaskBits()) : element.getEndAddress().getHostAddress());
         if (enableProxySelection)
         {
            if (ConsoleSharedData.getSession().isZoningEnabled())
            {
               zoneSelector.setZoneUIN(element.getZoneUIN());
            }
            proxySelector.setObjectId(element.getProxyId());
         }
         comment.setText(element.getComment());
      }
		
		return dialogArea;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.dialogs.Dialog#okPressed()
	 */
	@Override
	protected void okPressed()
	{
		try
		{
		   long zoneUIN = (zoneSelector != null) ? zoneSelector.getZoneUIN() : 0;
		   if (zoneUIN == -1)
		      zoneUIN = 0;
		   long proxyId = (proxySelector != null) ? proxySelector.getObjectId() : 0;
	      if (radioSubnet.getSelection())
	      {
	         InetAddress baseAddress = InetAddress.getByName(textAddr1.getText().trim());
	         int maskBits = Integer.parseInt(textAddr2.getText().trim());
	         if ((maskBits < 0) ||
	             ((baseAddress instanceof Inet4Address) && (maskBits > 32)) ||
                ((baseAddress instanceof Inet6Address) && (maskBits > 128)))
	            throw new NumberFormatException("Invalid network mask");
	         if(element == null)
	            element = new InetAddressListElement(baseAddress, maskBits, zoneUIN, proxyId, comment.getText());
	         else
	            element.update(baseAddress, maskBits, zoneUIN, proxyId, comment.getText());
	      }
	      else
	      {
	         if(element == null)
	            element = new InetAddressListElement(InetAddress.getByName(textAddr1.getText().trim()), InetAddress.getByName(textAddr2.getText().trim()), zoneUIN, proxyId, comment.getText());
	         else
               element.update(InetAddress.getByName(textAddr1.getText().trim()), InetAddress.getByName(textAddr2.getText().trim()), zoneUIN, proxyId, comment.getText());
	            
	      }
		}
		catch(UnknownHostException e)
		{
			MessageDialogHelper.openWarning(getShell(), Messages.get().AddAddressListElementDialog_Warning, Messages.get().AddAddressListElementDialog_EnterValidData);
			return;
		}
      catch(NumberFormatException e)
      {
         MessageDialogHelper.openWarning(getShell(), Messages.get().AddAddressListElementDialog_Warning, Messages.get().AddAddressListElementDialog_EnterValidData);
         return;
      }
		super.okPressed();
	}

   /**
    * @return the element
    */
   public InetAddressListElement getElement()
   {
      return element;
   }
}
