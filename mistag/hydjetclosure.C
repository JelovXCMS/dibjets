#include "../helpers/plotting.h"
#include "../helpers/looptuple.h"
#include "../helpers/config.h"
#include "../helpers/physics.h"

void checkclosure()
{
  vector<TH1F *>hsig(Nbins);
  vector<TH1F *>hasd(Nbins);
  vector<TH1F *>hbkg(Nbins);
  vector<TH1F *>hsub(Nbins);
  vector<TH1F *>hhyj(Nbins);
  vector<TH1F *>hshj(Nbins);
  vector<TH1F *>hsbn(Nbins);


  for (int i=0;i<Nbins;i++) {
    seth(10,0,1);
    hsig[i] = geth(Form("hsig%d",i),Form("Signal away-side %s;x_{J}",binnames[i].Data())) ;
    hasd[i] = geth(Form("hasd%d",i),Form("Measured away-side %s;x_{J}",binnames[i].Data()));
    hbkg[i] = geth(Form("hbkg%d",i),Form("Near-side %s;x_{J}",binnames[i].Data()));
    hhyj[i] = geth(Form("hhyj%d",i),Form("Near-side hydjet %s;x_{J}",binnames[i].Data()));
    hsub[i] = geth(Form("hsub%d",i),Form("Subtracted NS %s;x_{J}",binnames[i].Data()));
    hshj[i] = geth(Form("hshj%d",i),Form("Subtracted Hydjet %s;x_{J}",binnames[i].Data()));
    hsbn[i] = geth(Form("hsbn%d",i),Form("Subtracted Naive %s;x_{J}",binnames[i].Data()));
  }




  auto fmcPb = config.getfile_djt("mcPbbfa");

  Fill(fmcPb,{"pthat","weight","jtpt1","refpt1","bProdCode","jtptSL","refptSL","dphiSL1","refparton_flavorForB1","subidSL","bin","pairCodeSL1","discr_csvV1_1","jteta1","jtetaSL"},[&] (dict d) {
      if (d["pthat"]<pthatcut) return;
      
      if (d["jtpt1"]>pt1cut && d["refpt1"]>50 && abs(d["refparton_flavorForB1"])==5 && d["jtptSL"]>pt2cut) {
        int bin = getbinindex(d["bin"]);
        
        float xj = d["jtptSL"]/d["jtpt1"];
        float w = weight1SLPbPb(d);
        if (AwaySide(d)) hasd[bin]->Fill(xj, w);
        if (AwaySide(d) && IsSignal(d)) hsig[bin]->Fill(xj,w);

        if (NearSide(d)) hbkg[bin]->Fill(xj,w);
        if (NearSide(d) && !IsSignal(d)) hhyj[bin]->Fill(xj,w);
      }
        



      });



  for (int i=0;i<Nbins;i++) {
    hsub[i]->Add(hasd[i],hbkg[i],1,-1*bkgfractionInNearSide[i]);
    hsbn[i]->Add(hasd[i],hbkg[i],1,-1);
    hshj[i]->Add(hasd[i],hhyj[i],1,-1);
  }
//  for (int i=0;i<Nbins;i++) 
//    hincsub[i]->Add(hincasd[i],hincbkg[i],1,-1);

  seth(bins);//Nbins,0,100);
  auto hcentrSubSIG = geth("hcentrSubSIG","Signal;bin;#LTx_{J}#GT");
  auto hcentrSubASD = geth("hcentrSubASD","Unsubtracted;bin;#LTx_{J}#GT");

  auto hcentrSubBKS = geth("hcentrSubBKS","Subtracted w/o bkg scaling;bin;#LTx_{J}#GT");
  auto hcentrSubCLS = geth("hcentrSubCLS","Subtracted with bkg scaling;bin;#LTx_{J}#GT");
  auto hcentrSubHJS = geth("hcentrSubHJS","Subtracted Hydjet;bin;#LTx_{J}#GT");


  plotlegendpos = BottomRight;


  for (int i=0;i<Nbins;i++) {
    hcentrSubSIG->SetBinContent(i+1,hsig[i]->GetMean());hcentrSubSIG->SetBinError(i+1,hsig[i]->GetMeanError());
    hcentrSubASD->SetBinContent(i+1,hasd[i]->GetMean());hcentrSubASD->SetBinError(i+1,hasd[i]->GetMeanError());
    hcentrSubBKS->SetBinContent(i+1,hsbn[i]->GetMean());hcentrSubBKS->SetBinError(i+1,hsbn[i]->GetMeanError());

    hcentrSubCLS->SetBinContent(i+1,hsub[i]->GetMean());hcentrSubCLS->SetBinError(i+1,hsub[i]->GetMeanError());
    hcentrSubHJS->SetBinContent(i+1,hshj[i]->GetMean());hcentrSubHJS->SetBinError(i+1,hshj[i]->GetMeanError());

    Draw({hsig[i],hsub[i],hshj[i]});
  }


  plotymin = 0.55;//0.4;
  plotymax = 0.7;//0.8;
  plotlegendpos = BottomRight;
  aktstring = "";


  plotputmean = false;
  //hcentrSubHJS - hydjet only subtraction
  // SetMC({hcentrSubSIG, hcentrSubBKS, hcentrSubASD});
  // SetData({hcentrSubCLS});

  hcentrSubSIG->SetMarkerStyle(kOpenSquare);
  hcentrSubBKS->SetMarkerStyle(kOpenSquare);
  hcentrSubASD->SetMarkerStyle(kOpenSquare);
  hcentrSubCLS->SetMarkerStyle(kFullCircle);


  hcentrSubSIG->SetMarkerColor(TColor::GetColorDark(2)); hcentrSubSIG->SetLineColor(TColor::GetColorDark(2));
  hcentrSubBKS->SetMarkerColor(TColor::GetColorDark(3)); hcentrSubBKS->SetLineColor(TColor::GetColorDark(3));
  hcentrSubASD->SetMarkerColor(TColor::GetColorDark(4)); hcentrSubASD->SetLineColor(TColor::GetColorDark(4));
  hcentrSubCLS->SetMarkerColor(TColor::GetColorDark(3)); hcentrSubCLS->SetLineColor(TColor::GetColorDark(3));

  plotoverwritecolors = false;
  plotlegenddx = -0.15;

  Draw({hcentrSubSIG,hcentrSubASD, hcentrSubBKS, hcentrSubCLS});


  auto syst = (TH1F *)hcentrSubSIG->Clone("syst");
  syst->Add(hcentrSubCLS,-1);
  map<TString,float> m;
  for (unsigned i=0;i<bins.size()-1;i++) {
    float misclosure = syst->GetBinContent(i+1);
    float err = hcentrSubCLS->GetBinError(i+1);
    m[Form("closure%d%d",(int)bins[i],(int)bins[i+1])]=sqrt(misclosure*misclosure+err*err);
  }

  WriteToFile(plotfoldername+"/hydjetclosuresyst.root",m);


}



void hydjetclosure(bool firstRun = true)
{
  macro m("hydjetclosureplus10p",firstRun);
  checkclosure();
}