/*
** Copyright (C) 1998-2006 George Tzanetakis <gtzan@cs.uvic.ca>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/** 
    \class Series
    \brief Series of MarSystem objects

    Combines a series of MarSystem objects to a single MarSystem 
    corresponding to executing the System objects one after the other 
    in sequence. 
*/

#include "Series.h"

using namespace std;
using namespace Marsyas;

Series::Series(string name):MarSystem("Series",name)
{
  isComposite_ = true;
  addControls();
}

Series::~Series()
{
  deleteSlices();
}

MarSystem* 
Series::clone() const 
{
  return new Series(*this);
}

void 
Series::addControls()
{
}

void 
Series::deleteSlices()
{
  vector<realvec *>::const_iterator iter;
  for (iter= slices_.begin(); iter != slices_.end(); ++iter)
    {
      delete *(iter);
    }
  slices_.clear();
}

// STU
mrs_real* 
const Series::recvControls()
{
  if ( marsystemsSize_ != 0 ) {
    if (marsystems_[0]->getType() == "NetworkTCPSource" ) {
      return marsystems_[0]->recvControls();
    }
  }
  return 0;
}

void 
Series::myUpdate(MarControlPtr sender)
{

  if (marsystemsSize_ != 0) 
    {
      // update dataflow component MarSystems in order 
      marsystems_[0]->update();      

      for (mrs_natural i=1; i < marsystemsSize_; i++)
	{
	  //lmartins: replace updctrl() calls by setctrl()? ==> more efficient![?]
	  marsystems_[i]->updctrl("mrs_string/inObsNames", marsystems_[i-1]->getctrl("mrs_string/onObsNames"));
	  marsystems_[i]->updctrl("mrs_natural/inSamples", marsystems_[i-1]->getctrl("mrs_natural/onSamples"));
	  marsystems_[i]->updctrl("mrs_natural/inObservations", marsystems_[i-1]->getctrl("mrs_natural/onObservations"));
	  marsystems_[i]->updctrl("mrs_real/israte", marsystems_[i-1]->getctrl("mrs_real/osrate"));
	  marsystems_[i]->update();
	}

      // set controls based on first and last marsystem 
      setctrl("mrs_string/inObsNames", marsystems_[0]->getctrl("mrs_string/inObsNames"));
      setctrl("mrs_natural/inSamples", marsystems_[0]->getctrl("mrs_natural/inSamples"));
      setctrl("mrs_natural/inObservations", marsystems_[0]->getctrl("mrs_natural/inObservations"));
      setctrl("mrs_real/israte", marsystems_[0]->getctrl("mrs_real/israte"));

      setctrl("mrs_string/onObsNames", marsystems_[marsystemsSize_-1]->getctrl("mrs_string/onObsNames"));
      setctrl("mrs_natural/onSamples", marsystems_[marsystemsSize_-1]->getctrl("mrs_natural/onSamples")->toNatural());
      setctrl("mrs_natural/onObservations", marsystems_[marsystemsSize_-1]->getctrl("mrs_natural/onObservations")->toNatural());
      setctrl("mrs_real/osrate", marsystems_[marsystemsSize_-1]->getctrl("mrs_real/osrate")->toReal());

      // update buffers (aka slices) between components 
      if ((mrs_natural)slices_.size() < marsystemsSize_-1) 
	slices_.resize(marsystemsSize_-1, NULL);

      for (mrs_natural i=0; i< marsystemsSize_-1; i++)
	{
	  if (slices_[i] != NULL) 
	    {
	      if ((slices_[i])->getRows() != marsystems_[i]->getctrl("mrs_natural/onObservations")->toNatural()  ||
		  (slices_[i])->getCols() != marsystems_[i]->getctrl("mrs_natural/onSamples")->toNatural())
		{
		  delete slices_[i];
		  slices_[i] = new realvec(marsystems_[i]->getctrl("mrs_natural/onObservations")->toNatural(), 
					   marsystems_[i]->getctrl("mrs_natural/onSamples")->toNatural());
		  
		  (marsystems_[i])->updctrl("mrs_realvec/outTick", 
					  *(slices_[i]));
		  if (i > 0) 
		    {
		      marsystems_[i]->updctrl("mrs_realvec/inTick", 							   *(slices_[i]));		  
		    }
		  (slices_[i])->setval(0.0);
		}
	    }
	  else 
	    {
	      slices_[i] = new realvec(marsystems_[i]->getctrl("mrs_natural/onObservations")->toNatural(), 
				       marsystems_[i]->getctrl("mrs_natural/onSamples")->toNatural());
	      
	      marsystems_[i]->updctrl("mrs_realvec/outTick", 
				       *(slices_[i]));
	      if (i > 0) 
		marsystems_[i]->updctrl("mrs_realvec/inTick", 							   *(slices_[i]));
				
	      (slices_[i])->setval(0.0);
	    }
	}

      //defaultUpdate();      
    } 
}

void
Series::myProcess(realvec& in, realvec& out)
{
  //checkFlow(in,out);

  // Add assertions about sizes [!]

  if (marsystemsSize_ == 1)
    marsystems_[0]->process(in,out);
  else
    {
      for (mrs_natural i = 0; i < marsystemsSize_; i++)
	{
	  if (i==0)
	    {
	      marsystems_[i]->process(in, (realvec &) marsystems_[i]->getctrl("mrs_realvec/outTick")->to<mrs_realvec>());
	    }
	  else if (i == marsystemsSize_-1)
	    {
	      marsystems_[i]->process((realvec &) marsystems_[i-1]->getctrl("mrs_realvec/outTick")->to<mrs_realvec>(), out);
	    }
	  else
	    marsystems_[i]->process((realvec &) marsystems_[i-1]->getctrl("mrs_realvec/outTick")->to<mrs_realvec>(), (realvec &) marsystems_[i]->getctrl("mrs_realvec/outTick")->to<mrs_realvec>());
	  
	}
    }

}



