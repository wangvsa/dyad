.. _DYAD_pytorch:

*****************************************************
An Example of Integrating DYAD into PyToch DataLoader
*****************************************************

.. literalinclude:: demos/SCA26/DL/dyad_torch_data_loader.py
   :language: python
   :linenos:
   :caption: Unet3D DataLoader in DLIO

The lines relevant to DYAD are:

- The line 29 for importing PyDAYD
- The lines 71-84 for initialization
- The lines in ``__get_item__()``
- `dlp.` lines are for profiler and not directly relevant to DYAD
